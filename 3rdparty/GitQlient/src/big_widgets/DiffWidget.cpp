#include "DiffWidget.h"

#include <CommitInfoPanel.h>
#include <FileDiffWidget.h>
#include <FileListWidget.h>
#include <FullDiffWidget.h>
#include <GitCache.h>
#include <GitHistory.h>
#include <GitQlientSettings.h>

#include <QLogger.h>
#include <QPinnableTabWidget.h>

#include <QHBoxLayout>
#include <QMessageBox>

using namespace QLogger;

DiffWidget::DiffWidget(const QSharedPointer<GitBase> git, QSharedPointer<GitCache> cache, QWidget *parent)
   : QFrame(parent)
   , mGit(git)
   , mCache(cache)
   , mInfoPanelBase(new CommitInfoPanel())
   , mInfoPanelParent(new CommitInfoPanel())
   , mCenterStackedWidget(new QPinnableTabWidget())
   , fileListWidget(new FileListWidget(mGit, cache))
{
   setAttribute(Qt::WA_DeleteOnClose);

   mInfoPanelParent->setObjectName("InfoPanel");
   mInfoPanelParent->setFixedWidth(350);

   mCenterStackedWidget->setCurrentIndex(0);
   mCenterStackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   connect(mCenterStackedWidget, &QTabWidget::currentChanged, this, &DiffWidget::changeSelection);
   connect(mCenterStackedWidget, &QTabWidget::tabCloseRequested, this, &DiffWidget::onTabClosed);

   const auto wipSeparator = new QFrame();
   wipSeparator->setObjectName("separator");

   const auto infoPanel = new QFrame();
   infoPanel->setFixedWidth(350);
   infoPanel->setObjectName("InfoPanel");
   const auto diffsLayout = new QVBoxLayout(infoPanel);
   diffsLayout->setContentsMargins(QMargins());
   diffsLayout->setSpacing(0);
   diffsLayout->addWidget(mInfoPanelBase);
   diffsLayout->addWidget(wipSeparator);
   diffsLayout->addWidget(fileListWidget);

   const auto panelLayout = new QVBoxLayout();
   panelLayout->setContentsMargins(QMargins());
   panelLayout->setSpacing(0);
   panelLayout->addWidget(infoPanel);
   panelLayout->addStretch();
   panelLayout->addWidget(mInfoPanelParent);

   const auto layout = new QHBoxLayout();
   layout->setContentsMargins(QMargins());
   layout->addLayout(panelLayout);
   layout->setSpacing(10);
   layout->addWidget(mCenterStackedWidget);

   setLayout(layout);

   connect(fileListWidget, &FileListWidget::itemDoubleClicked, this, &DiffWidget::onDoubleClick);
   connect(fileListWidget, &FileListWidget::signalShowFileHistory, this, &DiffWidget::signalShowFileHistory);

   fileListWidget->setVisible(false);
}

DiffWidget::~DiffWidget()
{
   mDiffWidgets.clear();
   blockSignals(true);
}

void DiffWidget::reload()
{
   if (mCenterStackedWidget->count() > 0)
   {
      if (const auto fileDiff = dynamic_cast<FileDiffWidget *>(mCenterStackedWidget->currentWidget()))
         fileDiff->reload();
      else if (const auto fullDiff = dynamic_cast<FullDiffWidget *>(mCenterStackedWidget->currentWidget()))
         fullDiff->reload();
   }
}

void DiffWidget::clear() const
{
   mCenterStackedWidget->setCurrentIndex(0);
}

bool DiffWidget::loadFileDiff(const QString &currentSha, const QString &previousSha, const QString &file, bool isCached)
{
   const auto id = QString("%1 (%2 \u2194 %3)").arg(file.split("/").last(), currentSha.left(6), previousSha.left(6));

   mCurrentSha = currentSha;
   mParentSha = previousSha;

   if (!mDiffWidgets.contains(id))
   {
      QLog_Info(
          "UI",
          QString("Requested diff for file {%1} on between commits {%2} and {%3}").arg(file, currentSha, previousSha));

      const auto fileDiffWidget = new FileDiffWidget(mGit, mCache);
      const auto fileWithModifications = fileDiffWidget->configure(currentSha, previousSha, file, isCached);

      if (fileWithModifications)
      {
         mInfoPanelBase->configure(mCache->commitInfo(currentSha));
         mInfoPanelParent->configure(mCache->commitInfo(previousSha));

         mDiffWidgets.insert(id, fileDiffWidget);

         const auto index = mCenterStackedWidget->addTab(fileDiffWidget, file.split("/").last());
         mCenterStackedWidget->setCurrentIndex(index);

         fileListWidget->insertFiles(currentSha, previousSha);
         fileListWidget->setVisible(true);

         return true;
      }
      else
      {
         QMessageBox::information(this, tr("No modifications"), tr("There are no content modifications for this file"));
         delete fileDiffWidget;

         return false;
      }
   }
   else
   {
      const auto diffWidget = mDiffWidgets.value(id);
      const auto diff = dynamic_cast<FileDiffWidget *>(diffWidget);
      diff->reload();

      mCenterStackedWidget->setCurrentWidget(diff);

      return true;
   }
}

bool DiffWidget::loadCommitDiff(const QString &sha, const QString &parentSha)
{
   const auto id = QString("Commit diff (%1 \u2194 %2)").arg(sha.left(6), parentSha.left(6));

   mCurrentSha = sha;
   mParentSha = parentSha;

   if (!mDiffWidgets.contains(id))
   {
      QScopedPointer<GitHistory> git(new GitHistory(mGit));
      const auto ret = git->getCommitDiff(sha, parentSha);

      if (ret.success && !ret.output.isEmpty())
      {
         const auto fullDiffWidget = new FullDiffWidget(mGit, mCache);
         fullDiffWidget->loadDiff(sha, parentSha, ret.output);

         mInfoPanelBase->configure(mCache->commitInfo(sha));
         mInfoPanelParent->configure(mCache->commitInfo(parentSha));

         mDiffWidgets.insert(id, fullDiffWidget);

         const auto index = mCenterStackedWidget->addTab(fullDiffWidget,
                                                         QString("(%1 \u2194 %2)").arg(sha.left(6), parentSha.left(6)));
         mCenterStackedWidget->setCurrentIndex(index);

         fileListWidget->insertFiles(sha, parentSha);
         fileListWidget->setVisible(true);

         return true;
      }
      else
         QMessageBox::information(this, tr("No diff to show!"),
                                  tr("There is no diff to show between commit SHAs {%1} and {%2}").arg(sha, parentSha));

      return false;
   }
   else
   {
      const auto diffWidget = mDiffWidgets.value(id);
      const auto diff = dynamic_cast<FullDiffWidget *>(diffWidget);
      diff->reload();
      mCenterStackedWidget->setCurrentWidget(diff);
   }

   return true;
}

void DiffWidget::changeSelection(int index)
{
   const auto widget = qobject_cast<IDiffWidget *>(mCenterStackedWidget->widget(index));

   if (widget)
   {
      mInfoPanelBase->configure(mCache->commitInfo(widget->getCurrentSha()));
      mInfoPanelParent->configure(mCache->commitInfo(widget->getPreviousSha()));
   }
   else
      emit signalDiffEmpty();
}

void DiffWidget::onTabClosed(int index)
{
   const auto widget = qobject_cast<IDiffWidget *>(mCenterStackedWidget->widget(index));

   if (widget)
   {
      const auto key = mDiffWidgets.key(widget);
      mDiffWidgets.remove(key);
   }
}

void DiffWidget::onDoubleClick(QListWidgetItem *item)
{
   loadFileDiff(mCurrentSha, mParentSha, item->text(), false);
}
