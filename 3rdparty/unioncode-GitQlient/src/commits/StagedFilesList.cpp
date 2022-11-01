#include "StagedFilesList.h"

#include <GitQlientRole.h>

#include <QMenu>

StagedFilesList::StagedFilesList(QWidget *parent)
   : QListWidget(parent)
{
   connect(this, &QListWidget::customContextMenuRequested, this, &StagedFilesList::onContextMenu);
   connect(this, &QListWidget::itemDoubleClicked, this, &StagedFilesList::onDoubleClick);
}

void StagedFilesList::onContextMenu(const QPoint &pos)
{
   if (mSelectedItem = itemAt(pos); mSelectedItem != nullptr)
   {
      const auto menu = new QMenu(this);

      if (mSelectedItem->flags() & Qt::ItemIsSelectable)
      {
         const auto itemOriginalList = qvariant_cast<QListWidget *>(mSelectedItem->data(GitQlientRole::U_ListRole));

         if (sender() == itemOriginalList)
            connect(menu->addAction(tr("Reset")), &QAction::triggered, this, &StagedFilesList::onResetFile);
         else
            connect(menu->addAction(tr("See changes")), &QAction::triggered, this, &StagedFilesList::onShowDiff);
      }

      menu->popup(mapToGlobal(mapToParent(pos)));
   }
}

void StagedFilesList::onResetFile()
{
   emit signalResetFile(mSelectedItem);
}

void StagedFilesList::onShowDiff()
{
   emit signalShowDiff(mSelectedItem->toolTip());
}

void StagedFilesList::onDoubleClick(QListWidgetItem *item)
{
   emit signalShowDiff(item->toolTip());
}
