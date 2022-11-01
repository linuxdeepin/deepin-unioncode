#include <LineNumberArea.h>

#include <FileDiffView.h>
#include <GitQlientStyles.h>
#include <Colors.h>

#include <QPainter>
#include <QTextBlock>
#include <QIcon>

LineNumberArea::LineNumberArea(FileDiffView *editor, bool allowComments)
   : QWidget(editor)
   , mCommentsAllowed(allowComments)
{
   fileDiffWidget = editor;
   setMouseTracking(true);
}

int LineNumberArea::widthInDigitsSize()
{
   return 3;
}

QSize LineNumberArea::sizeHint() const
{
   return { fileDiffWidget->lineNumberAreaWidth(), 0 };
}

void LineNumberArea::setEditor(FileDiffView *editor)
{
   fileDiffWidget = editor;
   setParent(editor);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);

   const auto fontWidth = fileDiffWidget->fontMetrics().horizontalAdvance(QLatin1Char(' '));
   const auto offset = fontWidth * (mCommentsAllowed ? 4 : 1);
   auto block = fileDiffWidget->firstVisibleBlock();
   auto blockNumber = block.blockNumber() + fileDiffWidget->mStartingLine;
   auto top = fileDiffWidget->blockBoundingGeometry(block).translated(fileDiffWidget->contentOffset()).top();
   auto bottom = top + fileDiffWidget->blockBoundingRect(block).height();
   auto lineCorrection = 0;

   while (block.isValid() && top <= event->rect().bottom())
   {

      if (block.isVisible() && bottom >= event->rect().top())
      {
         const auto skipDeletion
             = fileDiffWidget->mUnified && !block.text().startsWith("-") && !block.text().startsWith("@");

         if (!fileDiffWidget->mUnified || skipDeletion)
         {
            const auto height = fileDiffWidget->fontMetrics().height();
            const auto number = blockNumber + 1 + lineCorrection;
            painter.setPen(GitQlientStyles::getTextColor());

            if (mBookmarks.contains(number))
            {
               painter.drawPixmap(6, static_cast<int>(top), height, height,
                                  QIcon(":/icons/comments").pixmap(height, height));

               painter.setPen(gitQlientOrange);
            }
            else if (fileDiffWidget->mRow == number)
            {
               painter.drawPixmap(width() - height - fontWidth, static_cast<int>(top), height, height,
                                  QIcon(":/icons/add_comment").pixmap(height, height));

               painter.setPen(gitQlientOrange);
            }

            painter.drawText(0, static_cast<int>(top), width() - offset, height, Qt::AlignRight,
                             QString::number(number));
         }
         else
            --lineCorrection;
      }

      block = block.next();
      top = bottom;
      bottom = top + fileDiffWidget->blockBoundingRect(block).height();
      ++blockNumber;
   }
}

void LineNumberArea::mouseMoveEvent(QMouseEvent *e)
{
   if (mCommentsAllowed)
   {
      if (rect().contains(e->pos()))
      {
         const auto height = width();
         const auto helpPos = mapFromGlobal(QCursor::pos());
         const auto x = helpPos.x();
         if (x >= 0 && x <= height)
         {
            QTextCursor cursor = fileDiffWidget->cursorForPosition(helpPos);
            const auto textRow = cursor.block().blockNumber();
            auto found = false;

            for (const auto &diff : qAsConst(fileDiffWidget->mFileDiffInfo))
            {
               if (textRow + 1 >= diff.startLine && textRow + 1 <= diff.endLine)
               {
                  fileDiffWidget->mRow = textRow + fileDiffWidget->mStartingLine + 1;
                  found = true;
                  break;
               }
            }

            if (!found)
               fileDiffWidget->mRow = -1;

            repaint();
         }
      }
      else
      {
         fileDiffWidget->mRow = -1;
         repaint();
      }
   }
}

void LineNumberArea::mousePressEvent(QMouseEvent *e)
{
   if (mCommentsAllowed)
      mPressed = rect().contains(e->pos());
}

void LineNumberArea::mouseReleaseEvent(QMouseEvent *e)
{
   if (mCommentsAllowed && mPressed && rect().contains(e->pos()))
   {
      const auto height = width();
      const auto helpPos = mapFromGlobal(QCursor::pos());
      const auto x = helpPos.x();
      if (x >= 0 && x <= height)
      {
         const auto cursor = fileDiffWidget->cursorForPosition(helpPos);
         const auto position = cursor.block().blockNumber() + 1;
         const auto row = position + fileDiffWidget->mStartingLine;
         const auto linkId = mBookmarks.value(row, -1);

         if (linkId == -1)
            emit addComment(row);
         else
            emit gotoReview(linkId);
      }
   }

   mPressed = false;
}
