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

#include <Issue.h>
#include <GitServerCache.h>
#include <document.h>

#include <QFrame>
#include <QMutex>

namespace GitServer
{
struct Issue;
struct PullRequest;
struct Comment;
struct Review;
struct CodeReview;
}

class QLabel;
class QVBoxLayout;
class QNetworkAccessManager;
class QHBoxLayout;
class QScrollArea;
class QTextEdit;

class HighlightningFrame : public QFrame
{
   Q_OBJECT
   Q_PROPERTY(QColor color READ color WRITE setColor)

public:
   explicit HighlightningFrame(QWidget *parent = nullptr);

   void setColor(QColor color);
   QColor color();
};

class PrCommentsList : public QFrame
{
   Q_OBJECT

signals:
   void frameReviewLink(GitServer::PullRequest pr, const QMap<int, int> &links);

public:
   enum class Config
   {
      Issues,
      PullRequests
   };

   explicit PrCommentsList(const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent = nullptr);
   ~PrCommentsList();

   void loadData(Config config, int issueNumber);
   void highlightComment(int frameId);
   void addGlobalComment();

private:
   QMutex mMutex;
   QSharedPointer<GitServerCache> mGitServerCache = nullptr;
   QNetworkAccessManager *mManager = nullptr;
   QFrame *mCommentsFrame = nullptr;
   QVBoxLayout *mIssuesLayout = nullptr;
   QFrame *mIssuesFrame = nullptr;
   QFrame *mInputFrame = nullptr;
   QTextEdit *mInputTextEdit = nullptr;
   Config mConfig {};
   QScrollArea *mScroll = nullptr;
   bool mLoaded = false;
   int mIssueNumber = -1;
   QMap<int, QFrame *> mComments {};
   QMap<int, int> mFrameLinks {};
   inline static int mCommentId = 0;
   Document m_content;
   QVector<Document *> m_commentContents;

   void processComments(const GitServer::Issue &issue);
   QLabel *createHeadline(const QDateTime &dt, const QString &prefix = QString());
   void onReviewsReceived();
   QLayout *createBubbleForComment(const GitServer::Comment &comment);
   QLayout *createBubbleForReview(const GitServer::Review &review);
   QVector<QLayout *> createBubbleForCodeReview(int reviewId, QVector<GitServer::CodeReview> &comments);
   void addReplyToCodeReview(int commentId, const QString &message);
};
