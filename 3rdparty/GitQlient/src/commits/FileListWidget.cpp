#include "FileListWidget.h"

#include <FileContextMenu.h>
#include <FileListDelegate.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitHistory.h>
#include <GitQlientStyles.h>
#include <RevisionFiles.h>

#include <QApplication>
#include <QDrag>
#include <QItemDelegate>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>

FileListWidget::FileListWidget(const QSharedPointer<GitBase> &git, QSharedPointer<GitCache> cache, QWidget *p)
   : QListWidget(p)
   , mGit(git)
   , mCache(std::move(cache))
{
   setContextMenuPolicy(Qt::CustomContextMenu);
   setItemDelegate(mFileDelegate = new FileListDelegate(this));
   setAttribute(Qt::WA_DeleteOnClose);

   connect(this, &FileListWidget::customContextMenuRequested, this, &FileListWidget::showContextMenu);
}

FileListWidget::~FileListWidget()
{
   delete mFileDelegate;
}

void FileListWidget::addItem(const QString &label, const QColor &clr)
{
   const auto item = new QListWidgetItem(label, this);
   item->setForeground(clr);
   item->setToolTip(label);
}

void FileListWidget::showContextMenu(const QPoint &pos)
{
   const auto item = itemAt(pos);

   if (item)
   {
      const auto fileName = item->data(Qt::DisplayRole).toString();
      const auto menu = new FileContextMenu(fileName, mCurrentSha == CommitInfo::ZERO_SHA, this);
      connect(menu, &FileContextMenu::signalShowFileHistory, this,
              [this, fileName]() { emit signalShowFileHistory(fileName); });
      connect(menu, &FileContextMenu::signalOpenFileDiff, this,
              [this, item] { emit QListWidget::itemDoubleClicked(item); });
      connect(menu, &FileContextMenu::signalEditFile, this,
              [this, fileName]() { emit signalEditFile(mGit->getWorkingDir() + "/" + fileName, 0, 0); });
      menu->exec(viewport()->mapToGlobal(pos));
   }
}

void FileListWidget::insertFiles(const QString &currentSha, const QString &compareToSha)
{
   clear();

   mCurrentSha = currentSha;

   auto files = mCache->revisionFile(mCurrentSha, compareToSha);

   if (!files)
   {
      QScopedPointer<GitHistory> git(new GitHistory(mGit));
      const auto ret = git->getDiffFiles(mCurrentSha, compareToSha);

      if (ret.success)
      {
         files = RevisionFiles(ret.output);
         mCache->insertRevisionFiles(mCurrentSha, compareToSha, files.value());
      }
   }

   if (files->count() != 0)
   {
      setUpdatesEnabled(false);

      for (auto i = 0; i < files->count(); ++i)
      {
         if (!files->statusCmp(i, RevisionFiles::UNKNOWN))
         {
            QColor clr;
            QString fileName;

            if (files->statusCmp(i, RevisionFiles::NEW))
            {
               const auto fileRename = files->extendedStatus(i);

               clr = fileRename.isEmpty() ? GitQlientStyles::getGreen() : GitQlientStyles::getBlue();
               fileName = fileRename.isEmpty() ? files->getFile(i) : fileRename;
            }
            else
            {
               clr = files->statusCmp(i, RevisionFiles::DELETED) ? GitQlientStyles::getRed()
                                                                 : GitQlientStyles::getTextColor();
               fileName = files->getFile(i);
            }

            addItem(fileName, clr);
         }
      }

      setUpdatesEnabled(true);
   }
}
