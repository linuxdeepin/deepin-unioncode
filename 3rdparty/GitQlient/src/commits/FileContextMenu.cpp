#include "FileContextMenu.h"

#include <QApplication>
#include <QClipboard>

FileContextMenu::FileContextMenu(const QString &file, bool editionAllowed, QWidget *parent)
   : QMenu(parent)
{
   setAttribute(Qt::WA_DeleteOnClose);

   const auto fileHistoryAction = addAction(tr("History"));
   fileHistoryAction->setEnabled(false);

   connect(addAction(tr("Blame")), &QAction::triggered, this, &FileContextMenu::signalShowFileHistory);

   const auto fileDiffAction = addAction(tr("Diff"));
   connect(fileDiffAction, &QAction::triggered, this, &FileContextMenu::signalOpenFileDiff);

   addSeparator();

   if (editionAllowed)
   {
      connect(addAction(tr("Edit file")), &QAction::triggered, this, &FileContextMenu::signalEditFile);

      addSeparator();
   }

   const auto copyPathAction = addAction(tr("Copy path"));
   connect(copyPathAction, &QAction::triggered, this, [file]() { QApplication::clipboard()->setText(file); });
}
