#include "FullDiffWidget.h"

#include <CommitInfo.h>
#include <DiffHelper.h>
#include <GitCache.h>
#include <GitHistory.h>
#include <GitQlientStyles.h>

#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCharFormat>
#include <QTextCodec>
#include <QVBoxLayout>

FullDiffWidget::DiffHighlighter::DiffHighlighter(QTextDocument *document)
   : QSyntaxHighlighter(document)
{
}

void FullDiffWidget::DiffHighlighter::highlightBlock(const QString &text)
{
   // state is used to count paragraphs, starting from 0
   setCurrentBlockState(previousBlockState() + 1);
   if (text.isEmpty())
      return;

   QTextCharFormat myFormat;
   const char firstChar = text.at(0).toLatin1();
   switch (firstChar)
   {
      case '@':
         myFormat.setForeground(GitQlientStyles::getOrange());
         myFormat.setFontWeight(QFont::ExtraBold);
         break;
      case '+':
         myFormat.setForeground(GitQlientStyles::getGreen());
         break;
      case '-':
         myFormat.setForeground(GitQlientStyles::getRed());
         break;
      case 'c':
      case 'd':
      case 'i':
      case 'n':
      case 'o':
      case 'r':
      case 's':
         if (text.startsWith("diff --git a/"))
         {
            myFormat.setForeground(GitQlientStyles::getBlue());
            myFormat.setFontWeight(QFont::ExtraBold);
         }
         else if (text.startsWith("copy ") || text.startsWith("index ") || text.startsWith("new ")
                  || text.startsWith("old ") || text.startsWith("rename ") || text.startsWith("similarity "))
            myFormat.setForeground(GitQlientStyles::getBlue());
         break;
      default:
         break;
   }
   if (myFormat.isValid())
      setFormat(0, text.length(), myFormat);
}

FullDiffWidget::FullDiffWidget(const QSharedPointer<GitBase> &git, QSharedPointer<GitCache> cache, QWidget *parent)
   : IDiffWidget(git, cache, parent)
   , mGoPrevious(new QPushButton())
   , mGoNext(new QPushButton())
   , mDiffWidget(new QPlainTextEdit())
{
   setAttribute(Qt::WA_DeleteOnClose);

   diffHighlighter = new DiffHighlighter(mDiffWidget->document());

   QFont font;
   font.setFamily(QString::fromUtf8("DejaVu Sans Mono"));
   mDiffWidget->setFont(font);
   mDiffWidget->setObjectName("textEditDiff");
   mDiffWidget->setUndoRedoEnabled(false);
   mDiffWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
   mDiffWidget->setReadOnly(true);
   mDiffWidget->setTextInteractionFlags(Qt::TextSelectableByMouse);

   const auto search = new QLineEdit();
   search->setPlaceholderText(tr("Press Enter to search a text... "));
   search->setObjectName("SearchInput");
   connect(search, &QLineEdit::editingFinished, this,
           [this, search]() { DiffHelper::findString(search->text(), mDiffWidget, this); });

   const auto optionsLayout = new QHBoxLayout();
   optionsLayout->setContentsMargins(QMargins());
   optionsLayout->setSpacing(5);
   optionsLayout->addWidget(mGoPrevious);
   optionsLayout->addWidget(mGoNext);
   optionsLayout->addStretch();

   const auto layout = new QVBoxLayout(this);
   layout->setContentsMargins(10, 10, 10, 10);
   layout->setSpacing(10);
   layout->addLayout(optionsLayout);
   layout->addWidget(search);
   layout->addWidget(mDiffWidget);

   mGoPrevious->setIcon(QIcon(":/icons/arrow_up"));
   mGoPrevious->setToolTip(tr("Previous change"));
   connect(mGoPrevious, &QPushButton::clicked, this, &FullDiffWidget::moveChunkUp);

   mGoNext->setToolTip(tr("Next change"));
   mGoNext->setIcon(QIcon(":/icons/arrow_down"));
   connect(mGoNext, &QPushButton::clicked, this, &FullDiffWidget::moveChunkDown);
}

bool FullDiffWidget::reload()
{
   if (mCurrentSha != CommitInfo::ZERO_SHA)
   {
      QScopedPointer<GitHistory> git(new GitHistory(mGit));
      const auto ret = git->getCommitDiff(mCurrentSha, mPreviousSha);

      if (ret.success && !ret.output.isEmpty())
      {
         loadDiff(mCurrentSha, mPreviousSha, ret.output);
         return true;
      }
   }

   return false;
}

void FullDiffWidget::processData(const QString &fileChunk)
{
   if (mPreviousDiffText != fileChunk)
   {
      mPreviousDiffText = fileChunk;

      auto count = 0;
      const auto chunks = fileChunk.split("\n");

      for (const auto &chunk : chunks)
      {
         if (chunk.startsWith("diff --"))
            mFilePositions.append(count);

         ++count;
      }

      const auto pos = mDiffWidget->verticalScrollBar()->value();

      mDiffWidget->setUpdatesEnabled(false);
      mDiffWidget->clear();
      mDiffWidget->setPlainText(fileChunk);
      mDiffWidget->moveCursor(QTextCursor::Start);
      mDiffWidget->verticalScrollBar()->setValue(pos);
      mDiffWidget->setUpdatesEnabled(true);
   }
}

void FullDiffWidget::moveChunkUp()
{
   const auto currentPos = mDiffWidget->verticalScrollBar()->value();

   const auto iter = std::find_if(mFilePositions.crbegin(), mFilePositions.crend(),
                                  [currentPos](int pos) { return currentPos > pos; });

   if (iter != mFilePositions.crend())
   {
      blockSignals(true);
      mDiffWidget->verticalScrollBar()->setValue(*iter);
      blockSignals(false);
   }
}

void FullDiffWidget::moveChunkDown()
{
   const auto currentPos = mDiffWidget->verticalScrollBar()->value();

   const auto iter = std::find_if(mFilePositions.cbegin(), mFilePositions.cend(),
                                  [currentPos](int pos) { return currentPos < pos; });

   if (iter != mFilePositions.cend())
   {
      blockSignals(true);
      mDiffWidget->verticalScrollBar()->setValue(*iter);
      blockSignals(false);
   }
}

void FullDiffWidget::loadDiff(const QString &sha, const QString &diffToSha, const QString &diffData)
{
   mCurrentSha = sha;
   mPreviousSha = diffToSha;

   processData(diffData);
}
