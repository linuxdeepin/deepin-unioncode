#include <CommitChangesWidget.h>
#include <ui_CommitChangesWidget.h>

#include <ClickableFrame.h>
#include <CommitInfo.h>
#include <FileWidget.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitLocal.h>
#include <GitQlientSettings.h>
#include <GitQlientStyles.h>
#include <GitRepoLoader.h>
#include <GitWip.h>
#include <RevisionFiles.h>
#include <UnstagedMenu.h>

#include <QDir>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QRegExp>
#include <QScrollBar>
#include <QTextCodec>
#include <QTextStream>
#include <QToolTip>

#include <QLogger.h>

using namespace QLogger;

QString CommitChangesWidget::lastMsgBeforeError;

enum GitQlientRole
{
   U_ListRole = Qt::UserRole,
   U_IsConflict,
   U_IsUntracked,
   U_FullPath
};

CommitChangesWidget::CommitChangesWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                                         QWidget *parent)
   : QWidget(parent)
   , ui(new Ui::CommitChangesWidget)
   , mCache(cache)
   , mGit(git)
{
   ui->setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);

   ui->amendFrame->setVisible(false);

   mTitleMaxLength = GitQlientSettings().globalValue("commitTitleMaxLength", mTitleMaxLength).toInt();

   ui->lCounter->setText(QString::number(mTitleMaxLength));
   ui->leCommitTitle->setMaxLength(mTitleMaxLength);
   ui->teDescription->setMaximumHeight(100);

   connect(ui->leCommitTitle, &QLineEdit::textChanged, this, &CommitChangesWidget::updateCounter);
   connect(ui->leCommitTitle, &QLineEdit::returnPressed, this, &CommitChangesWidget::commitChanges);
   connect(ui->applyActionBtn, &QPushButton::clicked, this, &CommitChangesWidget::commitChanges);
   connect(ui->warningButton, &QPushButton::clicked, this, [this]() { emit signalCancelAmend(mCurrentSha); });
   connect(ui->stagedFilesList, &StagedFilesList::signalResetFile, this, &CommitChangesWidget::resetFile);
   connect(ui->stagedFilesList, &StagedFilesList::signalShowDiff, this,
           [this](const QString &fileName) { requestDiff(mGit->getWorkingDir() + "/" + fileName); });
   connect(ui->unstagedFilesList, &QListWidget::customContextMenuRequested, this,
           &CommitChangesWidget::showUnstagedMenu);
   connect(ui->unstagedFilesList, &QListWidget::itemDoubleClicked, this,
           [this](QListWidgetItem *item) { requestDiff(mGit->getWorkingDir() + "/" + item->toolTip()); });

   ui->warningButton->setVisible(false);
   ui->applyActionBtn->setText(tr("Commit"));
}

CommitChangesWidget::~CommitChangesWidget()
{
   delete ui;
}

void CommitChangesWidget::reload()
{
   configure(mCurrentSha);
}

void CommitChangesWidget::resetFile(QListWidgetItem *item)
{
   QScopedPointer<GitLocal> git(new GitLocal(mGit));
   const auto ret = git->resetFile(item->toolTip());
   const auto revInfo = mCache->commitInfo(mCurrentSha);
   const auto files = mCache->revisionFile(mCurrentSha, revInfo.firstParent());

   for (auto i = 0; i < files->count(); ++i)
   {
      auto fileName = files->getFile(i);

      if (fileName == item->toolTip())
      {
         const auto isUnknown = files->statusCmp(i, RevisionFiles::UNKNOWN);
         const auto isInIndex = files->statusCmp(i, RevisionFiles::IN_INDEX);
         const auto untrackedFile = !isInIndex && isUnknown;
         const auto row = ui->stagedFilesList->row(item);
         const auto iconPath = QString(":/icons/add");
         const auto fileWidget = qobject_cast<FileWidget *>(ui->stagedFilesList->itemWidget(item));

         QFontMetrics metrix(item->font());
         const auto clippedText = metrix.elidedText(item->toolTip(), Qt::ElideMiddle, width() - 10);

         if (isInIndex || untrackedFile)
         {
            item->setData(GitQlientRole::U_ListRole, QVariant::fromValue(ui->unstagedFilesList));

            ui->stagedFilesList->takeItem(row);
            ui->unstagedFilesList->addItem(item);

            const auto newFileWidget = new FileWidget(iconPath, clippedText, this);
            newFileWidget->setTextColor(fileWidget->getTextColor());
            newFileWidget->setToolTip(fileName);

            connect(newFileWidget, &FileWidget::clicked, this, [this, item]() { addFileToCommitList(item); });
            ui->unstagedFilesList->setItemWidget(item, newFileWidget);

            delete fileWidget;
         }
      }
   }

   if (ret.success)
      emit signalUpdateWip();
}

QColor CommitChangesWidget::getColorForFile(const RevisionFiles &files, int index) const
{
   const auto isUnknown = files.statusCmp(index, RevisionFiles::UNKNOWN);
   const auto isInIndex = files.statusCmp(index, RevisionFiles::IN_INDEX);
   const auto isConflict = files.statusCmp(index, RevisionFiles::CONFLICT);
   const auto untrackedFile = !isInIndex && isUnknown;
   const auto isPartiallyCached = files.statusCmp(index, RevisionFiles::PARTIALLY_CACHED);

   QColor myColor;
   const auto isDeleted = files.statusCmp(index, RevisionFiles::DELETED);

   if (isConflict)
      myColor = GitQlientStyles::getBlue();
   else if (isDeleted)
      myColor = GitQlientStyles::getRed();
   else if (untrackedFile)
      myColor = GitQlientStyles::getOrange();
   else if (files.statusCmp(index, RevisionFiles::NEW) || isUnknown || isInIndex || isPartiallyCached)
      myColor = GitQlientStyles::getGreen();
   else
      myColor = GitQlientStyles::getTextColor();

   return myColor;
}

void CommitChangesWidget::deleteUntrackedFiles()
{
   for (auto i = 0; i < ui->unstagedFilesList->count(); ++i)
   {
      const auto item = ui->unstagedFilesList->item(i);

      if (item->data(GitQlientRole::U_IsUntracked).toBool())
      {
         const auto path = QString("%1").arg(item->data(GitQlientRole::U_FullPath).toString());

         QLog_Info("UI", "Removing path: " + path);

         QProcess p;
         p.setWorkingDirectory(mGit->getWorkingDir());
         p.start("rm", { "-rf", path });

         p.waitForFinished();
      }
   }

   emit signalCheckoutPerformed();
}

void CommitChangesWidget::prepareCache()
{
   for (auto it = mInternalCache.begin(); it != mInternalCache.end(); ++it)
      it.value().keep = false;
}

void CommitChangesWidget::clearCache()
{

   for (auto it = mInternalCache.begin(); it != mInternalCache.end();)
   {
      if (!it.value().keep)
      {
         if (it.value().item)
            delete it.value().item;

         it = mInternalCache.erase(it);
      }
      else
         ++it;
   }
}

void CommitChangesWidget::insertFiles(const RevisionFiles &files, QListWidget *fileList)
{
   for (auto &cachedItem : mInternalCache) // Move to prepareCache
      cachedItem.keep = false;

   for (auto i = 0; i < files.count(); ++i)
   {
      const auto fileName = files.getFile(i);
      const auto isUnknown = files.statusCmp(i, RevisionFiles::UNKNOWN);
      const auto isInIndex = files.statusCmp(i, RevisionFiles::IN_INDEX);
      const auto isConflict = files.statusCmp(i, RevisionFiles::CONFLICT);
      const auto isPartiallyCached = files.statusCmp(i, RevisionFiles::PARTIALLY_CACHED);
      const auto staged = isInIndex && !isUnknown && !isConflict;
      const auto untrackedFile = !isInIndex && isUnknown;
      auto wip = QString("%1-%2").arg(fileName, ui->stagedFilesList->objectName());

      if (staged || isPartiallyCached)
      {
         if (!mInternalCache.contains(wip))
         {
            const auto color = getColorForFile(files, i);
            const auto itemPair = fillFileItemInfo(fileName, isConflict, untrackedFile, QString(":/icons/remove"),
                                                   color, ui->stagedFilesList);
            connect(itemPair.second, &FileWidget::clicked, this, [this, item = itemPair.first]() { resetFile(item); });

            ui->stagedFilesList->setItemWidget(itemPair.first, itemPair.second);
            itemPair.first->setSizeHint(itemPair.second->sizeHint());
         }
         else
            mInternalCache[wip].keep = true;
      }

      if (!staged)
      {
         wip = QString("%1-%2").arg(fileName, fileList->objectName());
         if (!mInternalCache.contains(wip))
         {
            // Item configuration
            auto color = getColorForFile(files, i);

            // If the item is not new but the color is green this is not correct.
            // It means that the file was partially staged so the color backs to default.
            if (!files.statusCmp(i, RevisionFiles::NEW) && color == GitQlientStyles::getGreen())
               color = GitQlientStyles::getTextColor();

            const auto itemPair
                = fillFileItemInfo(fileName, isConflict, untrackedFile, QString(":/icons/add"), color, fileList);

            connect(itemPair.second, &FileWidget::clicked, this,
                    [this, item = itemPair.first]() { addFileToCommitList(item); });

            fileList->setItemWidget(itemPair.first, itemPair.second);
            itemPair.first->setSizeHint(itemPair.second->sizeHint());
         }
         else
            mInternalCache[wip].keep = true;
      }
   }
}

QPair<QListWidgetItem *, FileWidget *> CommitChangesWidget::fillFileItemInfo(const QString &file, bool isConflict,
                                                                             bool isUntracked, const QString &icon,
                                                                             const QColor &color, QListWidget *parent)
{
   auto modName = file;
   auto item = new QListWidgetItem(parent);

   item->setData(GitQlientRole::U_FullPath, file);

   if (isConflict)
   {
      modName = QString(file + " (conflicts)");

      item->setData(GitQlientRole::U_IsConflict, isConflict);
   }

   item->setData(GitQlientRole::U_ListRole, QVariant::fromValue(parent));
   item->setData(GitQlientRole::U_IsUntracked, isUntracked);
   item->setToolTip(modName);

   QFontMetrics metrix(item->font());
   const auto clippedText = metrix.elidedText(modName, Qt::ElideMiddle, width() - 10);

   const auto fileWidget = new FileWidget(icon, clippedText, this);
   fileWidget->setTextColor(color);
   fileWidget->setToolTip(modName);

   mInternalCache.insert(QString("%1-%2").arg(file, parent->objectName()), { true, item });

   return qMakePair(item, fileWidget);
}

void CommitChangesWidget::addAllFilesToCommitList()
{
   QStringList files;

   for (auto i = ui->unstagedFilesList->count() - 1; i >= 0; --i)
      files += addFileToCommitList(ui->unstagedFilesList->item(i), false);

   const auto git = QScopedPointer<GitLocal>(new GitLocal(mGit));

   if (const auto ret = git->markFilesAsResolved(files); ret.success)
   {
      QScopedPointer<GitWip> git(new GitWip(mGit, mCache));
      git->updateWip();
   }

   ui->applyActionBtn->setEnabled(ui->stagedFilesList->count() > 0);
}

void CommitChangesWidget::requestDiff(const QString &fileName)
{
   const auto isCached = qobject_cast<StagedFilesList *>(sender()) == ui->stagedFilesList;
   emit signalShowDiff(CommitInfo::ZERO_SHA, mCache->commitInfo(CommitInfo::ZERO_SHA).firstParent(), fileName,
                       isCached);
}

QString CommitChangesWidget::addFileToCommitList(QListWidgetItem *item, bool updateGit)
{
   const auto fileList = qvariant_cast<QListWidget *>(item->data(GitQlientRole::U_ListRole));
   const auto fileWidget = qobject_cast<FileWidget *>(fileList->itemWidget(item));
   const auto fileName = fileWidget->toolTip().remove(tr("(conflicts)")).trimmed();

   if (updateGit)
   {
      const auto git = QScopedPointer<GitLocal>(new GitLocal(mGit));

      if (const auto ret = git->stageFile(fileName); ret.success)
      {
         QScopedPointer<GitWip> git(new GitWip(mGit, mCache));
         git->updateWip();
      }
   }

   const auto row = fileList->row(item);
   fileList->removeItemWidget(item);
   fileList->takeItem(row);

   const auto newKey = QString("%1-%2").arg(fileName, ui->stagedFilesList->objectName());

   if (!mInternalCache.contains(newKey))
   {
      const auto wip = mInternalCache.take(QString("%1-%2").arg(fileName, fileList->objectName()));
      mInternalCache.insert(newKey, wip);

      QFontMetrics metrix(item->font());
      const auto clippedText = metrix.elidedText(fileName, Qt::ElideMiddle, width() - 10);

      const auto newFileWidget = new FileWidget(":/icons/remove", clippedText, this);
      newFileWidget->setTextColor(fileWidget->getTextColor());
      newFileWidget->setToolTip(fileName);

      connect(newFileWidget, &FileWidget::clicked, this, [this, item]() { removeFileFromCommitList(item); });

      ui->stagedFilesList->addItem(item);
      ui->stagedFilesList->setItemWidget(item, newFileWidget);

      if (item->data(GitQlientRole::U_IsConflict).toBool())
      {
         newFileWidget->setText(fileName);
      }
   }

   delete fileWidget;

   ui->applyActionBtn->setEnabled(true);

   return fileName;
}

void CommitChangesWidget::revertAllChanges()
{
   auto needsUpdate = false;

   for (auto i = ui->unstagedFilesList->count() - 1; i >= 0; --i)
   {
      QScopedPointer<GitLocal> git(new GitLocal(mGit));
      needsUpdate |= git->checkoutFile(ui->unstagedFilesList->takeItem(i)->toolTip());
   }

   if (needsUpdate)
      emit signalCheckoutPerformed();
}

void CommitChangesWidget::removeFileFromCommitList(QListWidgetItem *item)
{
   if (item->flags() & Qt::ItemIsSelectable)
   {
      const auto itemOriginalList = qvariant_cast<QListWidget *>(item->data(GitQlientRole::U_ListRole));
      const auto row = ui->stagedFilesList->row(item);
      const auto fileWidget = qobject_cast<FileWidget *>(ui->stagedFilesList->itemWidget(item));
      const auto fileName = fileWidget->toolTip();

      const auto wip = mInternalCache.take(QString("%1-%2").arg(fileName, ui->stagedFilesList->objectName()));
      mInternalCache.insert(QString("%1-%2").arg(fileName, itemOriginalList->objectName()), wip);

      QFontMetrics metrix(item->font());
      const auto clippedText = metrix.elidedText(fileName, Qt::ElideMiddle, width() - 10);

      const auto newFileWidget = new FileWidget(":/icons/add", clippedText, this);
      newFileWidget->setTextColor(fileWidget->getTextColor());
      newFileWidget->setToolTip(fileName);

      connect(newFileWidget, &FileWidget::clicked, this, [this, item]() { addFileToCommitList(item); });

      QScopedPointer<GitLocal> git = QScopedPointer<GitLocal>(new GitLocal(mGit));
      git->resetFile(fileName);

      if (item->data(GitQlientRole::U_IsConflict).toBool())
      {
         newFileWidget->setText(fileName + tr(" (conflicts)"));
      }

      delete fileWidget;

      ui->stagedFilesList->removeItemWidget(item);
      const auto item = ui->stagedFilesList->takeItem(row);

      itemOriginalList->addItem(item);
      itemOriginalList->setItemWidget(item, newFileWidget);

      ui->applyActionBtn->setDisabled(ui->stagedFilesList->count() == 0);
   }
}

QStringList CommitChangesWidget::getFiles()
{
   QStringList selFiles;
   const auto totalItems = ui->stagedFilesList->count();

   for (auto i = 0; i < totalItems; ++i)
   {
      const auto fileWidget = static_cast<FileWidget *>(ui->stagedFilesList->itemWidget(ui->stagedFilesList->item(i)));
      selFiles.append(fileWidget->toolTip());
   }

   return selFiles;
}

bool CommitChangesWidget::checkMsg(QString &msg)
{
   const auto title = ui->leCommitTitle->text();

   if (title.isEmpty())
      QMessageBox::warning(this, "Commit changes", "Please, add a title.");

   msg = title;

   if (!ui->teDescription->toPlainText().isEmpty())
   {
      auto description = QString("\n\n%1").arg(ui->teDescription->toPlainText());
      description.remove(QRegExp("(^|\\n)\\s*#[^\\n]*")); // strip comments
      msg += description;
   }

   msg.replace(QRegExp("[ \\t\\r\\f\\v]+\\n"), "\n"); // strip line trailing cruft
   msg = msg.trimmed();

   if (msg.isEmpty())
   {
      QMessageBox::warning(this, "Commit changes", "Please, add a title.");
      return false;
   }

   msg = QString("%1\n%2\n")
             .arg(msg.section('\n', 0, 0, QString::SectionIncludeTrailingSep), msg.section('\n', 1).trimmed());

   return true;
}

void CommitChangesWidget::updateCounter(const QString &text)
{
   ui->lCounter->setText(QString::number(mTitleMaxLength - text.count()));
}

bool CommitChangesWidget::hasConflicts()
{
   for (const auto &iter : qAsConst(mInternalCache))
      if (iter.item->data(GitQlientRole::U_IsConflict).toBool())
         return true;

   return false;
}

void CommitChangesWidget::clear()
{
   ui->unstagedFilesList->clear();
   ui->stagedFilesList->clear();
   mInternalCache.clear();
   ui->leCommitTitle->clear();
   ui->teDescription->clear();
   ui->applyActionBtn->setEnabled(false);
}

void CommitChangesWidget::clearStaged()
{
   ui->stagedFilesList->clear();

   const auto end = mInternalCache.end();
   for (auto it = mInternalCache.begin(); it != end;)
   {
      if (it.key().contains(QString("-%1").arg(ui->stagedFilesList->objectName())))
         it = mInternalCache.erase(it);
      else
         ++it;
   }

   ui->applyActionBtn->setEnabled(false);
}

void CommitChangesWidget::setCommitTitleMaxLength()
{
   mTitleMaxLength = GitQlientSettings().globalValue("commitTitleMaxLength", mTitleMaxLength).toInt();

   ui->lCounter->setText(QString::number(mTitleMaxLength));
   ui->leCommitTitle->setMaxLength(mTitleMaxLength);
   updateCounter(ui->leCommitTitle->text());
}

void CommitChangesWidget::showUnstagedMenu(const QPoint &pos)
{
   const auto item = ui->unstagedFilesList->itemAt(pos);

   if (item)
   {
      const auto fileName = item->toolTip();
      const auto contextMenu = new UnstagedMenu(mGit, fileName, this);
      connect(contextMenu, &UnstagedMenu::signalEditFile, this, &CommitChangesWidget::signalEditFile);
      connect(contextMenu, &UnstagedMenu::signalShowDiff, this, &CommitChangesWidget::requestDiff);
      connect(contextMenu, &UnstagedMenu::signalCommitAll, this, &CommitChangesWidget::addAllFilesToCommitList);
      connect(contextMenu, &UnstagedMenu::signalRevertAll, this, &CommitChangesWidget::revertAllChanges);
      connect(contextMenu, &UnstagedMenu::changeReverted, this, &CommitChangesWidget::changeReverted);
      connect(contextMenu, &UnstagedMenu::signalCheckedOut, this, &CommitChangesWidget::signalCheckoutPerformed);
      connect(contextMenu, &UnstagedMenu::signalShowFileHistory, this, &CommitChangesWidget::signalShowFileHistory);
      connect(contextMenu, &UnstagedMenu::signalStageFile, this, [this, item] { addFileToCommitList(item); });
      connect(contextMenu, &UnstagedMenu::deleteUntracked, this, &CommitChangesWidget::deleteUntrackedFiles);

      const auto parentPos = ui->unstagedFilesList->mapToParent(pos);
      contextMenu->popup(mapToGlobal(parentPos));
   }
}
