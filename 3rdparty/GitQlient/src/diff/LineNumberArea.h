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

#include <QWidget>

#include <QMap>

class FileDiffView;

class LineNumberArea : public QWidget
{
   Q_OBJECT

signals:
   void gotoReview(int linkId);
   void addComment(int row);

public:
   using BookmarkLine = int;
   using LinkId = int;

   LineNumberArea(FileDiffView *editor, bool allowComments = false);

   int widthInDigitsSize();
   QSize sizeHint() const override;
   void setEditor(FileDiffView *editor);
   bool commentsAllowed() const { return mCommentsAllowed; }
   void setCommentBookmarks(const QMap<BookmarkLine, LinkId> &bookmarks) { mBookmarks = bookmarks; }

protected:
   void paintEvent(QPaintEvent *event) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;

private:
   FileDiffView *fileDiffWidget;
   bool mPressed = false;
   bool mCommentsAllowed = false;
   QMap<BookmarkLine, LinkId> mBookmarks;
};
