#include "PrChangeListItem.h"

#include <DiffHelper.h>
#include <FileDiffView.h>
#include <LineNumberArea.h>
#include <AddCodeReviewDialog.h>

#include <QGridLayout>
#include <QLabel>

using namespace DiffHelper;

PrChangeListItem::PrChangeListItem(DiffChange change, QWidget *parent)
   : QFrame(parent)
   , mNewFileStartingLine(change.newFileStartLine)
   , mOldFileName(change.oldFileName)
   , mNewFileName(change.newFileName)
{
   setObjectName("PrChangeListItem");

   DiffHelper::processDiff(change.content, change.newData, change.oldData);

   const auto fileName = change.oldFileName == change.newFileName
       ? change.newFileName
       : QString("%1 -> %2").arg(change.oldFileName, change.newFileName);

   const auto fileNameLabel = new QLabel(fileName);
   fileNameLabel->setObjectName("ChangeFileName");

   const auto headerLayout = new QVBoxLayout();
   headerLayout->setContentsMargins(QMargins());
   headerLayout->addWidget(fileNameLabel);

   const auto headerFrame = new QFrame();
   headerFrame->setObjectName("ChangeHeaderFrame");
   headerFrame->setLayout(headerLayout);

   const auto mainLayout = new QVBoxLayout(this);
   mainLayout->setContentsMargins(QMargins());
   mainLayout->setSpacing(0);
   mainLayout->addWidget(headerFrame);

   if (!change.header.isEmpty())
   {
      const auto headerLabel = new QLabel(change.header);
      headerLabel->setObjectName("ChangeHeader");
      headerLayout->addWidget(headerLabel);

      mNewFileEndingLine = mNewFileStartingLine + change.newData.first.count();

      const auto oldFile = new FileDiffView();
      const auto numberArea = new LineNumberArea(oldFile, true);
      numberArea->setObjectName("LineNumberArea");
      numberArea->setEditor(oldFile);
      connect(numberArea, &LineNumberArea::addComment, this, &PrChangeListItem::openReviewDialog);

      oldFile->setStartingLine(change.oldFileStartLine - 1);
      oldFile->loadDiff(change.oldData.first.join("\n").trimmed(), change.oldData.second);
      oldFile->addNumberArea(numberArea);
      oldFile->setMinimumWidth(590);
      oldFile->show();
      oldFile->setMinimumHeight(oldFile->getHeight());

      mNewFileDiff = new FileDiffView();
      mNewNumberArea = new LineNumberArea(mNewFileDiff, true);
      mNewNumberArea->setObjectName("LineNumberArea");
      mNewNumberArea->setEditor(mNewFileDiff);
      connect(mNewNumberArea, &LineNumberArea::addComment, this, &PrChangeListItem::openReviewDialog);

      mNewFileDiff->setStartingLine(change.newFileStartLine - 1);
      mNewFileDiff->loadDiff(change.newData.first.join("\n").trimmed(), change.newData.second);
      mNewFileDiff->addNumberArea(mNewNumberArea);
      mNewFileDiff->setMinimumWidth(590);
      mNewFileDiff->show();
      mNewFileDiff->setMinimumHeight(mNewFileDiff->getHeight());

      const auto diffLayout = new QHBoxLayout();
      diffLayout->setContentsMargins(QMargins());
      diffLayout->setSpacing(5);
      diffLayout->addWidget(oldFile);
      diffLayout->addWidget(mNewFileDiff);
      mainLayout->addLayout(diffLayout);

      connect(mNewNumberArea, &LineNumberArea::gotoReview, this, &PrChangeListItem::gotoReview);
   }
}

void PrChangeListItem::setBookmarks(const QMap<int, int> &bookmarks)
{
   mNewNumberArea->setCommentBookmarks(bookmarks);
}

void PrChangeListItem::openReviewDialog(int line)
{
   const auto path = qobject_cast<LineNumberArea *>(sender()) == mNewNumberArea ? mNewFileName : mOldFileName;

   const auto dlg = new AddCodeReviewDialog(ReviewMode::Comment);
   dlg->setWindowFlag(Qt::FramelessWindowHint);
   dlg->setWindowFlag(Qt::Tool);
   dlg->setWindowModality(Qt::ApplicationModal);
   dlg->setModal(true);

   auto pos = QCursor::pos();
   pos.setY(pos.y() + mNewFileDiff->getLineHeigth());

   dlg->move(pos);

   connect(dlg, &AddCodeReviewDialog::accepted, this,
           [this, line, path, dlg]() { emit addCodeReview(line, path, dlg->getText().trimmed()); });

   dlg->exec();
}
