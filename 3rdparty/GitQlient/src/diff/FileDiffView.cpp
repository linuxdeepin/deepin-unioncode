/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2020 Francesc M.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "FileDiffView.h"

#include <FileDiffHighlighter.h>
#include <LineNumberArea.h>

#include <QLogger.h>

#include <QScrollBar>
#include <QMenu>

using namespace QLogger;

FileDiffView::FileDiffView(QWidget *parent)
   : QPlainTextEdit(parent)
   , mDiffHighlighter(new FileDiffHighlighter(document()))
{
   setAttribute(Qt::WA_DeleteOnClose);
   setReadOnly(true);
   setContextMenuPolicy(Qt::CustomContextMenu);

   connect(this, &FileDiffView::customContextMenuRequested, this, &FileDiffView::showStagingMenu);
   connect(this, &FileDiffView::blockCountChanged, this, &FileDiffView::updateLineNumberAreaWidth);
   connect(this, &FileDiffView::updateRequest, this, &FileDiffView::updateLineNumberArea);
   connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FileDiffView::signalScrollChanged);
}

FileDiffView::~FileDiffView()
{
   delete mDiffHighlighter;
}

void FileDiffView::addNumberArea(LineNumberArea *numberArea)
{
   mLineNumberArea = numberArea;

   if (mLineNumberArea->commentsAllowed())
   {
      installEventFilter(this);
      setMouseTracking(true);
   }
}

void FileDiffView::loadDiff(const QString &text, const QVector<ChunkDiffInfo::ChunkInfo> &fileDiffInfo)
{
   QLog_Trace("UI",
              QString("FileDiffView::loadDiff - {%1} move scroll to pos {%2}")
                  .arg(objectName(), QString::number(verticalScrollBar()->value())));

   mFileDiffInfo = fileDiffInfo;

   mDiffHighlighter->setDiffInfo(mFileDiffInfo);

   const auto pos = verticalScrollBar()->value();
   auto cursor = textCursor();
   const auto tmpCursor = textCursor().position();
   setPlainText(text);

   cursor.setPosition(tmpCursor);
   setTextCursor(cursor);

   blockSignals(true);
   verticalScrollBar()->setValue(pos);
   blockSignals(false);

   emit updateRequest(viewport()->rect(), 0);

   QLog_Trace("UI",
              QString("FileDiffView::loadDiff - {%1} move scroll to pos {%2}").arg(objectName(), QString::number(pos)));
}

void FileDiffView::moveScrollBarToPos(int value)
{
   blockSignals(true);
   verticalScrollBar()->setValue(value);
   blockSignals(false);

   emit updateRequest(viewport()->rect(), 0);

   QLog_Trace("UI",
              QString("FileDiffView::moveScrollBarToPos - {%1} move scroll to pos {%2}")
                  .arg(objectName(), QString::number(value)));
}

int FileDiffView::getHeight() const
{
   auto block = firstVisibleBlock();
   auto height = 0;

   while (block.isValid())
   {
      height += blockBoundingRect(block).height();
      block = block.next();
   }

   return height;
}

int FileDiffView::getLineHeigth() const
{
   return blockBoundingRect(firstVisibleBlock()).height();
}

int FileDiffView::lineNumberAreaWidth()
{
   const auto width = fontMetrics().horizontalAdvance(QLatin1Char('9'));
   auto digits = mLineNumberArea ? mLineNumberArea->widthInDigitsSize() : 0;
   auto max = blockCount() + mStartingLine;

   while (max >= 10)
   {
      max /= 10;
      ++digits;
   }

   auto offset = 0;

   if (mLineNumberArea && mLineNumberArea->commentsAllowed())
   {
      const auto padding = 3;
      offset = (fontMetrics().height() * 2 + padding * 4);
   }

   return offset + (width * digits);
}

void FileDiffView::updateLineNumberAreaWidth(int /* newBlockCount */)
{
   setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void FileDiffView::updateLineNumberArea(const QRect &rect, int dy)
{
   if (mLineNumberArea)
   {
      if (dy != 0)
         mLineNumberArea->scroll(0, dy);
      else
         mLineNumberArea->update(0, rect.y(), mLineNumberArea->width(), rect.height());

      if (rect.contains(viewport()->rect()))
         updateLineNumberAreaWidth(0);
   }
}

void FileDiffView::resizeEvent(QResizeEvent *e)
{
   QPlainTextEdit::resizeEvent(e);

   if (mLineNumberArea)
   {
      const auto cr = contentsRect();

      mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
   }
}

bool FileDiffView::eventFilter(QObject *obj, QEvent *event)
{
   if (mLineNumberArea && event->type() == QEvent::Enter)
   {
      const auto height = mLineNumberArea->width();
      const auto helpPos = mapFromGlobal(QCursor::pos());
      const auto x = helpPos.x();
      if (x >= 0 && x <= height)
      {
         QTextCursor cursor = cursorForPosition(helpPos);
         const auto textRow = cursor.block().blockNumber();
         auto found = false;

         for (const auto &diff : qAsConst(mFileDiffInfo))
         {
            if (textRow + 1 >= diff.startLine && textRow + 1 <= diff.endLine)
            {
               mRow = textRow + mStartingLine + 1;
               found = true;
               break;
            }
         }

         if (!found)
            mRow = -1;

         repaint();
      }
   }
   else if (event->type() == QEvent::Leave)
   {
      mRow = -1;
      repaint();
   }

   return QPlainTextEdit::eventFilter(obj, event);
}

void FileDiffView::showStagingMenu(const QPoint &cursorPos)
{
   const auto helpPos = mapFromGlobal(QCursor::pos());
   const auto x = helpPos.x();
   auto row = -1;

   if (x >= 0 && x > mLineNumberArea->width())
   {
      const auto cursor = cursorForPosition(helpPos);
      row = cursor.block().blockNumber() + mStartingLine + 1;

      if (row != -1)
      {
         const auto chunk
             = std::find_if(mFileDiffInfo.cbegin(), mFileDiffInfo.cend(), [row](const ChunkDiffInfo::ChunkInfo &chunk) {
                  return chunk.startLine <= row && row <= chunk.endLine;
               });

         if (chunk != mFileDiffInfo.cend())
         {
            const auto menu = new QMenu(this);
            /*
            const auto stageLine = menu->addAction(tr("Stage line"));
            connect(stageLine, &QAction::triggered, this, &FileDiffView::stageLine);
            */

            const auto stageChunk = menu->addAction(tr("Stage chunk"));
            connect(stageChunk, &QAction::triggered, this,
                    [this, chunkId = chunk->id]() { emit signalStageChunk(chunkId); });

            menu->move(viewport()->mapToGlobal(cursorPos));
            menu->exec();
         }
      }
   }
}
