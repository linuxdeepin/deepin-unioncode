#include "BlameWidget.h"

#include <BranchesViewDelegate.h>
#include <CommitHistoryColumns.h>
#include <CommitHistoryModel.h>
#include <CommitHistoryView.h>
#include <CommitInfo.h>
#include <FileBlameWidget.h>
#include <GitHistory.h>
#include <RepositoryViewDelegate.h>

#include <QApplication>
#include <QClipboard>
#include <QFileSystemModel>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QTabWidget>
#include <QTreeView>

BlameWidget::BlameWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                         const QSharedPointer<GitQlientSettings> &settings, QWidget *parent)
   : QFrame(parent)
   , mCache(cache)
   , mGit(git)
   , mSettings(settings)
   , fileSystemModel(new QFileSystemModel())
   , mRepoModel(new CommitHistoryModel(mCache, mGit, nullptr))
   , mRepoView(new CommitHistoryView(mCache, mGit, mSettings, nullptr))
   , fileSystemView(new QTreeView())
   , mTabWidget(new QTabWidget())
{
   mTabWidget->setObjectName("HistoryTab");
   mRepoView->setObjectName("blameGraphView");
   mRepoView->setModel(mRepoModel);
   mRepoView->header()->setSectionHidden(static_cast<int>(CommitHistoryColumns::Graph), true);
   mRepoView->header()->setSectionHidden(static_cast<int>(CommitHistoryColumns::Date), true);
   mRepoView->header()->setSectionHidden(static_cast<int>(CommitHistoryColumns::Author), true);
   mRepoView->setItemDelegate(mItemDelegate = new RepositoryViewDelegate(cache, mGit, nullptr, mRepoView));
   mRepoView->setEnabled(true);
   mRepoView->setMaximumWidth(450);
   mRepoView->setSelectionBehavior(QAbstractItemView::SelectRows);
   mRepoView->setSelectionMode(QAbstractItemView::SingleSelection);
   mRepoView->setContextMenuPolicy(Qt::CustomContextMenu);
   mRepoView->header()->setContextMenuPolicy(Qt::NoContextMenu);
   mRepoView->activateFilter(true);
   mRepoView->filterBySha({});
   connect(mRepoView, &CommitHistoryView::customContextMenuRequested, this, &BlameWidget::showRepoViewMenu);
   connect(mRepoView, &CommitHistoryView::clicked, this, &BlameWidget::reloadBlame);
   connect(mRepoView, &CommitHistoryView::doubleClicked, this, &BlameWidget::openDiff);

   fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

   fileSystemView->setModel(fileSystemModel);
   fileSystemView->setMaximumWidth(450);
   fileSystemView->header()->setSectionHidden(1, true);
   fileSystemView->header()->setSectionHidden(2, true);
   fileSystemView->header()->setSectionHidden(3, true);
   fileSystemView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(fileSystemView, &QTreeView::clicked, this, &BlameWidget::showFileHistoryByIndex);

   const auto historyBlameLayout = new QGridLayout(this);
   historyBlameLayout->setContentsMargins(QMargins());
   historyBlameLayout->addWidget(mRepoView, 0, 0);
   historyBlameLayout->addWidget(fileSystemView, 1, 0);
   historyBlameLayout->addWidget(mTabWidget, 0, 1, 2, 1);

   mTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   connect(mTabWidget, &QTabWidget::tabCloseRequested, mTabWidget, [this](int index) {
      if (index == mLastTabIndex)
      {
         fileSystemView->clearSelection();
         mRepoView->blockSignals(true);
         mRepoView->filterBySha({});
         mRepoView->blockSignals(false);
      }

      auto widget = qobject_cast<FileBlameWidget *>(mTabWidget->widget(index));
      mTabWidget->removeTab(index);
      const auto key = mTabsMap.key(widget);
      mTabsMap.remove(key);

      delete widget;
   });
   connect(mTabWidget, &QTabWidget::currentChanged, this, &BlameWidget::reloadHistory);

   setAttribute(Qt::WA_DeleteOnClose);
}

BlameWidget::~BlameWidget()
{
   delete mRepoModel;
   delete mItemDelegate;
   delete fileSystemModel;
}

void BlameWidget::init(const QString &workingDirectory)
{
   mWorkingDirectory = workingDirectory;
   fileSystemModel->setRootPath(workingDirectory);
   fileSystemView->setRootIndex(fileSystemModel->index(workingDirectory));
}

void BlameWidget::showFileHistory(const QString &filePath)
{
   if (!mTabsMap.contains(filePath))
   {
      QScopedPointer<GitHistory> git(new GitHistory(mGit));
      auto ret = git->history(filePath);

      if (ret.success)
      {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
         auto shaHistory = ret.output.split("\n", Qt::SkipEmptyParts);
#else
         auto shaHistory = ret.output.split("\n", QString::SkipEmptyParts);
#endif
         for (auto i = 0; i < shaHistory.size();)
         {
            if (shaHistory.at(i).startsWith("gpg:"))
            {
               shaHistory.takeAt(i);

               if (shaHistory.size() <= i)
                  break;
            }
            else
               ++i;
         }

         mRepoView->blockSignals(true);
         mRepoView->filterBySha(shaHistory);
         mRepoView->blockSignals(false);

         const auto previousSha = shaHistory.count() > 1 ? shaHistory.at(1) : QString(tr("No info"));
         const auto fileBlameWidget = new FileBlameWidget(mCache, mGit);

         fileBlameWidget->setup(filePath, shaHistory.constFirst(), previousSha);
         connect(fileBlameWidget, &FileBlameWidget::signalCommitSelected, mRepoView, &CommitHistoryView::focusOnCommit);

         const auto index = mTabWidget->addTab(fileBlameWidget, filePath.split("/").last());
         mTabWidget->setTabsClosable(true);
         mTabWidget->blockSignals(true);
         mTabWidget->setCurrentIndex(index);
         mTabWidget->blockSignals(false);

         mLastTabIndex = index;
         mTabsMap.insert(filePath, fileBlameWidget);
      }
   }
   else
      mTabWidget->setCurrentWidget(mTabsMap.value(filePath));
}

void BlameWidget::onNewRevisions(int totalCommits)
{
   mRepoModel->onNewRevisions(totalCommits);
}

void BlameWidget::reloadBlame(const QModelIndex &index)
{
   mSelectedRow = index.row();
   const auto blameWidget = qobject_cast<FileBlameWidget *>(mTabWidget->currentWidget());

   if (blameWidget)
   {
      const auto sha
          = mRepoView->model()->index(index.row(), static_cast<int>(CommitHistoryColumns::Sha)).data().toString();
      const auto previousSha
          = mRepoView->model()->index(index.row() + 1, static_cast<int>(CommitHistoryColumns::Sha)).data().toString();
      blameWidget->reload(sha, previousSha);
   }
}

void BlameWidget::reloadHistory(int tabIndex)
{
   if (tabIndex >= 0)
   {
      mLastTabIndex = tabIndex;

      const auto blameWidget = qobject_cast<FileBlameWidget *>(mTabWidget->widget(tabIndex));
      const auto sha = blameWidget->getCurrentSha();
      const auto file = blameWidget->getCurrentFile();

      QScopedPointer<GitHistory> git(new GitHistory(mGit));
      const auto ret = git->history(file);

      if (ret.success)
      {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
         auto shaHistory = ret.output.split("\n", Qt::SkipEmptyParts);
#else
         auto shaHistory = ret.output.split("\n", QString::SkipEmptyParts);
#endif
         for (auto i = 0; i < shaHistory.size();)
         {
            if (shaHistory.at(i).startsWith("gpg:"))
            {
               shaHistory.takeAt(i);

               if (shaHistory.size() <= i)
                  break;
            }
            else
               ++i;
         }

         mRepoView->blockSignals(true);
         mRepoView->filterBySha(shaHistory);

         const auto repoModel = mRepoView->model();
         const auto totalRows = repoModel->rowCount();
         for (auto i = 0; i < totalRows; ++i)
         {
            const auto index = mRepoView->model()->index(i, static_cast<int>(CommitHistoryColumns::Sha));

            if (index.data().toString().startsWith(sha))
            {
               mRepoView->setCurrentIndex(index);
               mRepoView->selectionModel()->select(index,
                                                   QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
         }

         mRepoView->blockSignals(false);
      }
   }
}

void BlameWidget::showFileHistoryByIndex(const QModelIndex &index)
{
   auto item = fileSystemModel->fileInfo(index);

   if (item.isFile())
      showFileHistory(item.filePath());
}

void BlameWidget::showRepoViewMenu(const QPoint &pos)
{
   const auto shaColumnIndex = static_cast<int>(CommitHistoryColumns::Sha);
   const auto modelIndex = mRepoView->model()->index(mSelectedRow, shaColumnIndex);

   reloadBlame(modelIndex);

   const auto sha = modelIndex.data().toString();
   const auto previousSha = mRepoView->model()->index(mSelectedRow + 1, shaColumnIndex).data().toString();
   const auto menu = new QMenu(this);
   const auto copyShaAction = menu->addAction(tr("Copy SHA"));
   connect(copyShaAction, &QAction::triggered, this, [sha]() { QApplication::clipboard()->setText(sha); });

   const auto fileDiff = menu->addAction(tr("Show file diff"));
   connect(fileDiff, &QAction::triggered, this, [this, sha, previousSha]() {
      const auto currentFile = qobject_cast<FileBlameWidget *>(mTabWidget->currentWidget())->getCurrentFile();
      emit showFileDiff(sha, previousSha, currentFile, false);
   });

   const auto commitDiff = menu->addAction(tr("Show commit diff"));
   connect(commitDiff, &QAction::triggered, this, [this, sha, previousSha]() {
      emit signalOpenDiff({ previousSha, sha });
   });

   menu->exec(mRepoView->viewport()->mapToGlobal(pos));
}

void BlameWidget::openDiff(const QModelIndex &index)
{
   const auto sha
       = mRepoView->model()->index(index.row(), static_cast<int>(CommitHistoryColumns::Sha)).data().toString();
   const auto previousSha
       = mRepoView->model()->index(index.row() + 1, static_cast<int>(CommitHistoryColumns::Sha)).data().toString();

   emit signalOpenDiff({ previousSha, sha });
}
