#include "PrChangesList.h"

#include <DiffHelper.h>
#include <GitHistory.h>
#include <FileDiffView.h>
#include <PrChangeListItem.h>
#include <PullRequest.h>
#include <GitRemote.h>
#include <GitConfig.h>
#include <QLogger.h>

#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>

using namespace GitServer;
using namespace QLogger;

PrChangesList::PrChangesList(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QFrame(parent)
   , mGit(git)
{
   QScopedPointer<GitRemote> gitRemote(new GitRemote(mGit));
   gitRemote->fetch();
}

void PrChangesList::loadData(const GitServer::PullRequest &prInfo)
{
   GitExecResult ret;
   bool showDiff = true;
   QString head;

   if (prInfo.headRepo != prInfo.baseRepo)
   {
      QScopedPointer<GitConfig> git(new GitConfig(mGit));
      const auto ret = git->getGitValue(QString("remote.%1.url").arg(prInfo.headRepo.split("/").constFirst()));

      if (ret.output.isEmpty())
      {
         const auto response = QMessageBox::question(
             this, tr("Getting remote branch"),
             tr("The head branch of the Pull Request is not in the same repository. In order to show "
                "the changes the remote must be added. <b>Do you want to get the remote branch?</b>"));

         if (response == QMessageBox::Yes)
         {
            QScopedPointer<GitRemote> git(new GitRemote(mGit));
            const auto remoteAdded = git->addRemote(prInfo.headUrl, prInfo.headRepo.split("/").constFirst());

            showDiff = remoteAdded.success;

            if (!showDiff)
               QLog_Warning("UI", QString("Problems adding a remote: {%1}").arg(remoteAdded.output));
         }
         else
            showDiff = false;

         if (!showDiff)
            return;
      }

      head = QString("%1/%2").arg(prInfo.headRepo.split("/").constFirst(), prInfo.head);
   }
   else
   {
      QScopedPointer<GitConfig> git(new GitConfig(mGit));
      auto retBase = git->getRemoteForBranch(prInfo.head);
      head = QString("%1/%2").arg(retBase.success ? retBase.output : "origin", prInfo.head);
   }

   QScopedPointer<GitConfig> gitConfig(new GitConfig(mGit));
   auto retBase = gitConfig->getRemoteForBranch(prInfo.head);
   const auto base = QString("%1/%2").arg(retBase.success ? retBase.output : "origin", prInfo.base);

   QScopedPointer<GitHistory> git(new GitHistory(mGit));
   ret = git->getBranchesDiff(base, head);

   if (ret.success)
   {
      auto diff = ret.output;
      auto changes = DiffHelper::splitDiff(diff);

      if (!changes.isEmpty())
      {
         delete layout();

         const auto mainLayout = new QVBoxLayout();
         mainLayout->setContentsMargins(20, 20, 20, 20);
         mainLayout->setSpacing(0);

         for (auto &change : changes)
         {
            const auto changeListItem = new PrChangeListItem(change);
            connect(changeListItem, &PrChangeListItem::gotoReview, this, &PrChangesList::gotoReview);
            connect(changeListItem, &PrChangeListItem::addCodeReview, this, &PrChangesList::addCodeReview);

            mListItems.append(changeListItem);

            mainLayout->addWidget(changeListItem);
            mainLayout->addSpacing(10);
         }

         const auto mIssuesFrame = new QFrame();
         mIssuesFrame->setObjectName("IssuesViewFrame");
         mIssuesFrame->setLayout(mainLayout);

         const auto mScroll = new QScrollArea();
         mScroll->setWidgetResizable(true);
         mScroll->setWidget(mIssuesFrame);

         const auto aLayout = new QVBoxLayout(this);
         aLayout->setContentsMargins(QMargins());
         aLayout->setSpacing(0);
         aLayout->addWidget(mScroll);
      }
   }
}

void PrChangesList::onReviewsReceived(PullRequest pr)
{
   using Bookmark = QPair<int, int>;
   QMultiMap<QString, Bookmark> bookmarksPerFile;

   auto comments = pr.reviewComment;

   for (const auto &review : qAsConst(pr.reviews))
   {
      QMap<int, QVector<CodeReview>> reviews;
      QVector<int> codeReviewIds;

      auto iter = comments.begin();

      while (iter != comments.end())
      {
         if (iter->reviewId == review.id)
         {
            codeReviewIds.append(iter->id);
            reviews[iter->id].append(*iter);
            comments.erase(iter);
         }
         else if (codeReviewIds.contains(iter->replyToId))
         {
            reviews[iter->replyToId].append(*iter);
            comments.erase(iter);
         }
         else
            ++iter;
      }

      if (!reviews.isEmpty())
      {
         for (auto &codeReviews : reviews)
         {
            std::sort(codeReviews.begin(), codeReviews.end(),
                      [](const CodeReview &r1, const CodeReview &r2) { return r1.creation < r2.creation; });

            const auto first = codeReviews.constFirst();

            if (!first.outdated)
               bookmarksPerFile.insert(first.diff.file, { first.diff.line, first.id });
            else
               bookmarksPerFile.insert(first.diff.file, { first.diff.originalLine, first.id });
         }
      }
   }

   for (auto iter : qAsConst(mListItems))
   {
      QMap<int, int> bookmarks;
      const auto values = bookmarksPerFile.values(iter->getFileName());

      for (auto bookmark : qAsConst(values))
      {
         if (bookmark.first >= iter->getStartingLine() && bookmark.first <= iter->getEndingLine())
            bookmarks.insert(bookmark.first, bookmark.second);
      }

      if (!bookmarks.isEmpty())
         iter->setBookmarks(bookmarks);
   }
}

void PrChangesList::addLinks(PullRequest pr, const QMap<int, int> &reviewLinkToComments)
{
   QMultiMap<QString, QPair<int, int>> bookmarksPerFile;

   for (auto reviewId : reviewLinkToComments)
   {
      for (const auto &review : qAsConst(pr.reviewComment))
      {
         if (review.id == reviewId)
         {
            if (!review.outdated)
               bookmarksPerFile.insert(review.diff.file, { review.diff.line, review.id });
            else
               bookmarksPerFile.insert(review.diff.file, { review.diff.originalLine, review.id });

            break;
         }
      }
   }

   for (const auto &iter : qAsConst(mListItems))
   {
      QMap<int, int> bookmarks;

      const auto values = bookmarksPerFile.values(iter->getFileName());
      for (const auto &bookmark : values)
      {
         if (bookmark.first >= iter->getStartingLine() && bookmark.first <= iter->getEndingLine())
            bookmarks.insert(bookmark.first, reviewLinkToComments.key(bookmark.second));
      }

      if (!bookmarks.isEmpty())
         iter->setBookmarks(bookmarks);
   }
}
