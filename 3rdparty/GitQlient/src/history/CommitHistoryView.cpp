#include "CommitHistoryView.h"

#include <CommitHistoryColumns.h>
#include <CommitHistoryContextMenu.h>
#include <CommitHistoryModel.h>
#include <CommitInfo.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitConfig.h>
#include <GitQlientSettings.h>
#include <ShaFilterProxyModel.h>

#include <QDateTime>
#include <QHeaderView>

#include <QLogger.h>
using namespace QLogger;

CommitHistoryView::CommitHistoryView(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                                     const QSharedPointer<GitQlientSettings> &settings,
                                     const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent)
   : QTreeView(parent)
   , mCache(cache)
   , mGit(git)
   , mSettings(settings)
   , mGitServerCache(gitServerCache)
{
   setEnabled(false);
   setContextMenuPolicy(Qt::CustomContextMenu);
   setItemsExpandable(false);
   setMouseTracking(true);
   setSelectionMode(QAbstractItemView::ExtendedSelection);
   setAttribute(Qt::WA_DeleteOnClose);

   header()->setSortIndicatorShown(false);
   header()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(header(), &QHeaderView::customContextMenuRequested, this, &CommitHistoryView::onHeaderContextMenu);

   connect(mCache.get(), &GitCache::signalCacheUpdated, this, &CommitHistoryView::refreshView);

   connect(this, &CommitHistoryView::doubleClicked, this, [this](const QModelIndex &index) {
      if (mCommitHistoryModel)
      {
         const auto sha = mCommitHistoryModel->sha(index.row());
         emit signalOpenDiff(sha);
      }
   });
}

void CommitHistoryView::setModel(QAbstractItemModel *model)
{
   connect(this, &CommitHistoryView::customContextMenuRequested, this, &CommitHistoryView::showContextMenu,
           Qt::UniqueConnection);

   mCommitHistoryModel = dynamic_cast<CommitHistoryModel *>(model);
   QTreeView::setModel(model);
   setupGeometry();
   connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this,
           [this](const QItemSelection &selected, const QItemSelection &) {
              const auto indexes = selected.indexes();
              if (!indexes.isEmpty())
              {
                 scrollTo(indexes.first());
                 emit clicked(indexes.first());
              }
           });
}

void CommitHistoryView::filterBySha(const QStringList &shaList)
{
   mIsFiltering = true;

   if (mProxyModel)
   {
      mProxyModel->beginResetModel();
      mProxyModel->setAcceptedSha(shaList);
      mProxyModel->endResetModel();
   }
   else
   {
      mProxyModel = new ShaFilterProxyModel(this);
      mProxyModel->setSourceModel(mCommitHistoryModel);
      mProxyModel->setAcceptedSha(shaList);
      setModel(mProxyModel);
   }

   setupGeometry();
}

CommitHistoryView::~CommitHistoryView()
{
   mSettings->setLocalValue(QString("%1").arg(objectName()), header()->saveState());
}

void CommitHistoryView::setupGeometry()
{
   const auto previousState = mSettings->localValue(QString("%1").arg(objectName()), QByteArray()).toByteArray();

   if (previousState.isEmpty())
   {
      const auto hv = header();
      hv->setMinimumSectionSize(75);
      hv->resizeSection(static_cast<int>(CommitHistoryColumns::Sha), 75);
      hv->resizeSection(static_cast<int>(CommitHistoryColumns::Graph), 120);
      hv->resizeSection(static_cast<int>(CommitHistoryColumns::Author), 160);
      hv->resizeSection(static_cast<int>(CommitHistoryColumns::Date), 125);
      hv->resizeSection(static_cast<int>(CommitHistoryColumns::Sha), 75);
      hv->setSectionResizeMode(static_cast<int>(CommitHistoryColumns::Author), QHeaderView::Fixed);
      hv->setSectionResizeMode(static_cast<int>(CommitHistoryColumns::Date), QHeaderView::Fixed);
      hv->setSectionResizeMode(static_cast<int>(CommitHistoryColumns::Sha), QHeaderView::Fixed);
      hv->setSectionResizeMode(static_cast<int>(CommitHistoryColumns::Log), QHeaderView::Stretch);
      hv->setStretchLastSection(false);

      hideColumn(static_cast<int>(CommitHistoryColumns::TreeViewIcon));
   }
   else
   {
      header()->restoreState(previousState);
      header()->setSectionResizeMode(static_cast<int>(CommitHistoryColumns::Log), QHeaderView::Stretch);
   }
}

void CommitHistoryView::currentChanged(const QModelIndex &index, const QModelIndex &)
{
   mCurrentSha = model()->index(index.row(), static_cast<int>(CommitHistoryColumns::Sha)).data().toString();
}

void CommitHistoryView::refreshView()
{
   QModelIndex topLeft;
   QModelIndex bottomRight;

   if (mProxyModel)
   {
      topLeft = mProxyModel->index(0, 0);
      bottomRight = mProxyModel->index(mProxyModel->rowCount() - 1, mProxyModel->columnCount() - 1);
      mProxyModel->beginResetModel();
      mProxyModel->endResetModel();
   }
   else
   {
      topLeft = mCommitHistoryModel->index(0, 0);
      bottomRight
          = mCommitHistoryModel->index(mCommitHistoryModel->rowCount() - 1, mCommitHistoryModel->columnCount() - 1);
      mCommitHistoryModel->onNewRevisions(mCache->commitCount());
   }

   const auto auxTL = visualRect(topLeft);
   const auto auxBR = visualRect(bottomRight);
   viewport()->update(auxTL.x(), auxTL.y(), auxBR.x() + auxBR.width(), auxBR.y() + auxBR.height());
}

void CommitHistoryView::onHeaderContextMenu(const QPoint &pos)
{
   const auto menu = new QMenu(this);
   const auto total = header()->count();

   for (auto column = 3; column < total; ++column)
   {
      const auto columnName = model()->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
      const auto action = menu->addAction(columnName);
      const auto isHidden = isColumnHidden(column);
      action->setCheckable(true);
      action->setChecked(!isHidden);
      connect(action, &QAction::triggered, this, [column, this, action]() {
         action->setChecked(!action->isChecked());
         setColumnHidden(column, !isColumnHidden(column));
      });
   }

   menu->exec(header()->mapToGlobal(pos));
}

void CommitHistoryView::clear()
{
   mCommitHistoryModel->clear();
}

void CommitHistoryView::focusOnCommit(const QString &goToSha)
{
   mCurrentSha = goToSha;

   QLog_Info("UI", QString("Setting the focus on the commit {%1}").arg(mCurrentSha));

   auto row = mCache->commitInfo(mCurrentSha).pos;

   if (mIsFiltering)
   {
      const auto sourceIndex = mProxyModel->sourceModel()->index(row, 0);
      row = mProxyModel->mapFromSource(sourceIndex).row();
   }

   clearSelection();

   const auto index = model()->index(row, 0);

   setCurrentIndex(index);
   scrollTo(currentIndex());
}

QModelIndexList CommitHistoryView::selectedIndexes() const
{
   return QTreeView::selectedIndexes();
}

void CommitHistoryView::showContextMenu(const QPoint &pos)
{
   if (!mIsFiltering)
   {
      const auto shas = getSelectedShaList();

      if (!shas.isEmpty())
      {
         const auto menu = new CommitHistoryContextMenu(mCache, mGit, mGitServerCache, shas, this);
         connect(menu, &CommitHistoryContextMenu::fullReload, this, &CommitHistoryView::fullReload);
         connect(menu, &CommitHistoryContextMenu::referencesReload, this, &CommitHistoryView::referencesReload);
         connect(menu, &CommitHistoryContextMenu::logReload, this, &CommitHistoryView::logReload);
         connect(menu, &CommitHistoryContextMenu::signalOpenDiff, this, &CommitHistoryView::signalOpenDiff);
         connect(menu, &CommitHistoryContextMenu::signalOpenCompareDiff, this,
                 &CommitHistoryView::signalOpenCompareDiff);
         connect(menu, &CommitHistoryContextMenu::signalAmendCommit, this, &CommitHistoryView::signalAmendCommit);
         connect(menu, &CommitHistoryContextMenu::signalMergeRequired, this, &CommitHistoryView::signalMergeRequired);
         connect(menu, &CommitHistoryContextMenu::mergeSqushRequested, this, &CommitHistoryView::mergeSqushRequested);
         connect(menu, &CommitHistoryContextMenu::signalCherryPickConflict, this,
                 &CommitHistoryView::signalCherryPickConflict);
         connect(menu, &CommitHistoryContextMenu::signalPullConflict, this, &CommitHistoryView::signalPullConflict);
         connect(menu, &CommitHistoryContextMenu::showPrDetailedView, this, &CommitHistoryView::showPrDetailedView);
         menu->exec(viewport()->mapToGlobal(pos));
      }
      else
         QLog_Warning("UI", "SHAs selected belong to different branches. They need to share at least one branch.");
   }
}

QStringList CommitHistoryView::getSelectedShaList() const
{
   const auto indexes = selectedIndexes();

   if (indexes.count() > 0)
   {
      QMap<QDateTime, QString> shas;

      for (auto index : indexes)
      {
         const auto sha = mCommitHistoryModel->sha(index.row());
         const auto dtStr
             = mCommitHistoryModel->index(index.row(), static_cast<int>(CommitHistoryColumns::Date)).data().toString();

         shas.insert(QDateTime::fromString(dtStr, "dd MMM yyyy hh:mm"), sha);
      }

      return shas.values();
   }

   return QStringList();
}
