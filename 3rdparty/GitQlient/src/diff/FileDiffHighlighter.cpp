#include "FileDiffHighlighter.h"

#include <GitQlientStyles.h>
#include <QTextDocument>

FileDiffHighlighter::FileDiffHighlighter(QTextDocument *document)
   : QSyntaxHighlighter(document)
{
}

void FileDiffHighlighter::highlightBlock(const QString &text)
{
   setCurrentBlockState(previousBlockState() + 1);

   QTextBlockFormat myFormat;
   QTextCharFormat format;
   const auto currentLine = currentBlock().blockNumber() + 1;

   if (!mFileDiffInfo.isEmpty())
   {
      for (const auto &diff : qAsConst(mFileDiffInfo))
      {
         if (diff.startLine <= currentLine && currentLine <= diff.endLine)
         {
            if (diff.addition)
            {
               myFormat.setBackground(GitQlientStyles::getGreen());
               // myFormat.setForeground(GitQlientStyles::getGreen());
            }
            else
               myFormat.setBackground(GitQlientStyles::getRed());
         }
      }
   }
   else if (!text.isEmpty())
   {
      switch (text.at(0).toLatin1())
      {
         case '@':
            myFormat.setBackground(GitQlientStyles::getOrange());
            format.setFontWeight(QFont::ExtraBold);
            break;
         case '+':
            myFormat.setBackground(GitQlientStyles::getGreen());
            break;
         case '-':
            myFormat.setBackground(GitQlientStyles::getRed());
            break;
         default:
            break;
      }
   }

   if (myFormat.isValid())
   {
      QTextCursor(currentBlock()).setBlockFormat(myFormat);
      setFormat(0, currentBlock().length(), format);
   }
}
