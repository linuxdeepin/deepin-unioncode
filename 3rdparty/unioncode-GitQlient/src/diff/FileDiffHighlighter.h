#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <QSyntaxHighlighter>
#include <DiffInfo.h>

/*!
 \brief Overloaded class that adds syntax highlight for the diff view. It shows the additions in green, removals in red
 and the files where that happened in blue.

 \class FileDiffHighlighter FileDiffHighlighter.h "FileDiffHighlighter.h"
*/
class FileDiffHighlighter : public QSyntaxHighlighter
{
   Q_OBJECT

public:
   /*!
    \brief Default constructor.

    \param document The document to parse.
   */
   explicit FileDiffHighlighter(QTextDocument *document);

   /*!
    \brief Analyses a block of text and applies the syntax highlighter.

    \param text The block of text to analyse.
   */
   void highlightBlock(const QString &text) override;

   /**
    * @brief setDiffInfo Sets the file diff information that will be used to colour the foreground and background text.
    * @param fileDiffInfo The file diff information.
    */
   void setDiffInfo(const QVector<ChunkDiffInfo::ChunkInfo> &fileDiffInfo) { mFileDiffInfo = fileDiffInfo; }

private:
   QVector<ChunkDiffInfo::ChunkInfo> mFileDiffInfo;
};
