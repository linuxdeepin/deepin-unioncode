#include "PrCommentsList.h"

#include <GitServerCache.h>
#include <GitHubRestApi.h>
#include <GitLabRestApi.h>
#include <CircularPixmap.h>
#include <SourceCodeReview.h>
#include <AvatarHelper.h>
#include <CodeReviewComment.h>
#include <ButtonLink.hpp>
#include <Colors.h>
#include <previewpage.h>
#include <GitQlientSettings.h>

#include <QNetworkAccessManager>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QTextEdit>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QPushButton>
#include <QIcon>
#include <QScrollBar>
//#include <QWebChannel>
//#include <QWebEngineView>

using namespace GitServer;

PrCommentsList::PrCommentsList(const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent)
   : QFrame(parent)
   , mMutex(QMutex::Recursive)
   , mGitServerCache(gitServerCache)
   , mManager(new QNetworkAccessManager())
{
   setObjectName("IssuesViewFrame");
}

PrCommentsList::~PrCommentsList()
{
   delete mManager;
}

void PrCommentsList::loadData(PrCommentsList::Config config, int issueNumber)
{
   QMutexLocker lock(&mMutex);

   connect(mGitServerCache.get(), &GitServerCache::issueUpdated, this, &PrCommentsList::processComments,
           Qt::UniqueConnection);
   connect(mGitServerCache.get(), &GitServerCache::prReviewsReceived, this, &PrCommentsList::onReviewsReceived,
           Qt::UniqueConnection);

   mConfig = config;
   mIssueNumber = issueNumber;

   auto issue = config == Config::Issues ? mGitServerCache->getIssue(mIssueNumber)
                                         : mGitServerCache->getPullRequest(mIssueNumber);

   delete mIssuesFrame;
   delete mScroll;
   delete layout();

   mCommentsFrame = nullptr;

   mIssuesLayout = new QVBoxLayout();
   mIssuesLayout->setContentsMargins(QMargins());
   mIssuesLayout->setSpacing(30);

   mInputTextEdit = new QTextEdit();
   mInputTextEdit->setPlaceholderText(tr("Add your comment..."));
   mInputTextEdit->setObjectName("AddReviewInput");

   const auto cancel = new QPushButton(tr("Cancel"));
   const auto add = new QPushButton(tr("Comment"));
   connect(cancel, &QPushButton::clicked, this, [this]() {
      mInputTextEdit->clear();
      mInputFrame->setVisible(false);
   });

   connect(add, &QPushButton::clicked, this, [issue, this]() {
      connect(mGitServerCache.get(), &GitServerCache::issueUpdated, this, &PrCommentsList::processComments,
              Qt::UniqueConnection);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
      mGitServerCache->getApi()->addIssueComment(issue, mInputTextEdit->toMarkdown());
#else
      mGitServerCache->getApi()->addIssueComment(issue, mInputTextEdit->toPlainText());
#endif
      mInputTextEdit->clear();
      mInputFrame->setVisible(false);
   });

   const auto btnsLayout = new QHBoxLayout();
   btnsLayout->setContentsMargins(QMargins());
   btnsLayout->setSpacing(0);
   btnsLayout->addWidget(cancel);
   btnsLayout->addStretch();
   btnsLayout->addWidget(add);

   const auto inputLayout = new QVBoxLayout();
   inputLayout->setContentsMargins(20, 20, 20, 10);
   inputLayout->setSpacing(10);
   inputLayout->addWidget(mInputTextEdit);
   inputLayout->addLayout(btnsLayout);

   mInputFrame = new QFrame();
   mInputFrame->setFixedHeight(200);
   mInputFrame->setLayout(inputLayout);
   mInputFrame->setVisible(false);

   const auto descriptionFrame = new QFrame();
   descriptionFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

   const auto bodyLayout = new QVBoxLayout();
   bodyLayout->setContentsMargins(20, 20, 20, 20);
   bodyLayout->setAlignment(Qt::AlignTop);
   bodyLayout->setSpacing(30);
   bodyLayout->addWidget(descriptionFrame);
   bodyLayout->addLayout(mIssuesLayout);
   bodyLayout->addStretch();

   mIssuesFrame = new QFrame();
   mIssuesFrame->setObjectName("IssuesViewFrame");
   mIssuesFrame->setLayout(bodyLayout);

   mScroll = new QScrollArea();
   mScroll->setWidgetResizable(true);
   mScroll->setWidget(mIssuesFrame);

   const auto aLayout = new QVBoxLayout(this);
   aLayout->setContentsMargins(QMargins());
   aLayout->setSpacing(0);
   aLayout->addWidget(mScroll);
   aLayout->addWidget(mInputFrame);

   const auto creationLayout = new QHBoxLayout();
   creationLayout->setContentsMargins(QMargins());
   creationLayout->setSpacing(0);

   const auto days = issue.creation.daysTo(QDateTime::currentDateTime());
   const auto whenText = days <= 30
       ? days != 0 ? tr(" %1 days ago").arg(days) : tr(" today")
       : tr(" on %1").arg(issue.creation.date().toString(QLocale().dateFormat(QLocale::ShortFormat)));

   const auto creationLabel = new QLabel();
   creationLabel->setText(tr("<i>Created by <b>%1</b>%2</i> - ").arg(issue.creator.name, whenText));
   creationLabel->setToolTip(issue.creation.toString(QLocale().dateTimeFormat(QLocale::ShortFormat)));

   creationLayout->addWidget(creationLabel);

   if (!issue.assignees.isEmpty())
   {
      const auto assignedLayout = new QHBoxLayout();
      assignedLayout->setContentsMargins(QMargins());
      assignedLayout->setSpacing(0);

      assignedLayout->addWidget(new QLabel(tr("<i>Assigned to </i>")));

      auto count = 0;
      const auto totalAssignees = issue.assignees.count();
      for (auto &assignee : issue.assignees)
      {
         const auto assigneLabel = new QLabel(QString("<i><b>%1</b></i>").arg(assignee.name));
         assigneLabel->setObjectName("CreatorLink");
         assignedLayout->addWidget(assigneLabel);

         if (count++ < totalAssignees - 1)
            assignedLayout->addWidget(new QLabel(", "));
      }

      creationLayout->addLayout(assignedLayout);
   }
   else
      creationLayout->addWidget(new QLabel(tr("<i>Unassigned</i>")));

   creationLayout->addStretch();

   for (auto &label : issue.labels)
   {
      auto labelWidget = new QLabel();
      labelWidget->setStyleSheet(QString("QLabel {"
                                         "background-color: #%1;"
                                         "border-radius: 7px;"
                                         "min-height: 15px;"
                                         "max-height: 15px;"
                                         "min-width: 15px;"
                                         "max-width: 15px;}")
                                     .arg(label.colorHex));
      labelWidget->setToolTip(label.name);
      creationLayout->addWidget(labelWidget);
      creationLayout->addItem(new QSpacerItem(10, 1, QSizePolicy::Fixed, QSizePolicy::Fixed));
   }

   if (!issue.milestone.title.isEmpty())
   {
      const auto milestone = new QLabel(QString("%1").arg(issue.milestone.title));
      milestone->setObjectName("IssueLabel");
      creationLayout->addWidget(milestone);
   }

   const auto layout = new QVBoxLayout(descriptionFrame);
   layout->setContentsMargins(QMargins());
   layout->setSpacing(10);
   layout->addLayout(creationLayout);

   const auto bodyDescFrame = new QFrame();
   bodyDescFrame->setObjectName("IssueDescription");

   const auto bodyDescLayout = new QVBoxLayout(bodyDescFrame);
   bodyDescLayout->setContentsMargins(10, 10, 10, 10);

   GitQlientSettings settings("");
   const auto colorSchema = settings.globalValue("colorSchema", "dark").toString();
   const auto style = colorSchema == "dark" ? QString::fromUtf8("dark") : QString::fromUtf8("bright");

//   QPointer<QWebEngineView> body = new QWebEngineView();

//   PreviewPage *page = new PreviewPage(this);
//   body->setPage(page);

//   QWebChannel *channel = new QWebChannel(this);
//   channel->registerObject(QStringLiteral("content"), &m_content);
//   page->setWebChannel(channel);

//   body->setUrl(QUrl(QString("qrc:/resources/index_%1.html").arg(style)));
//   body->setFixedHeight(20);

//   connect(page, &PreviewPage::contentsSizeChanged, this, [body](const QSizeF size) {
//      if (body)
//         body->setFixedHeight(size.height());
//   });

   m_content.setText(QString::fromUtf8(issue.body));

//   bodyDescLayout->addWidget(body);
   layout->addWidget(bodyDescFrame);

   descriptionFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

   const auto separator = new QFrame();
   separator->setObjectName("orangeHSeparator");

   mIssuesLayout->addWidget(separator);

   if (mConfig == Config::Issues)
      mGitServerCache->getApi()->requestComments(issue.number);
   else
   {
      mGitServerCache->getApi()->requestComments(mIssueNumber);
      mGitServerCache->getApi()->requestReviews(mIssueNumber);
   }
}

void PrCommentsList::highlightComment(int frameId)
{
   const auto daFrame = mComments.value(frameId);

   mScroll->ensureWidgetVisible(daFrame);

   const auto animationGoup = new QSequentialAnimationGroup();
   auto animation = new QPropertyAnimation(daFrame, "color");
   animation->setDuration(500);
   animation->setStartValue(highlightCommentStart);
   animation->setEndValue(highlightCommentEnd);
   animationGoup->addAnimation(animation);

   animation = new QPropertyAnimation(daFrame, "color");
   animation->setDuration(500);
   animation->setStartValue(highlightCommentEnd);
   animation->setEndValue(highlightCommentStart);
   animationGoup->addAnimation(animation);

   animationGoup->start();
}

void PrCommentsList::addGlobalComment()
{
   mInputFrame->setVisible(true);
   mInputTextEdit->setFocus();
}

void PrCommentsList::processComments(const Issue &issue)
{
   QMutexLocker lock(&mMutex);

   disconnect(mGitServerCache.get(), &GitServerCache::issueUpdated, this, &PrCommentsList::processComments);

   if (mIssueNumber != issue.number)
      return;

   delete mCommentsFrame;

   mCommentsFrame = new QFrame();

   mIssuesLayout->addWidget(mCommentsFrame);

   const auto commentsLayout = new QVBoxLayout(mCommentsFrame);
   commentsLayout->setContentsMargins(QMargins());
   commentsLayout->setSpacing(30);

   for (auto &comment : issue.comments)
   {
      const auto layout = createBubbleForComment(comment);
      commentsLayout->addLayout(layout);
   }

   commentsLayout->addStretch();
}

QLabel *PrCommentsList::createHeadline(const QDateTime &dt, const QString &prefix)
{
   const auto days = dt.daysTo(QDateTime::currentDateTime());
   const auto whenText = days <= 30 ? days != 0 ? tr(" %1 days ago").arg(days) : tr(" today")
                                    : tr(" on %1").arg(dt.date().toString(QLocale().dateFormat(QLocale::ShortFormat)));

   const auto label = prefix.isEmpty() ? new QLabel(whenText) : new QLabel(prefix + whenText);
   label->setToolTip(dt.toString(QLocale().dateFormat(QLocale::ShortFormat)));

   return label;
}

void PrCommentsList::onReviewsReceived()
{
   QMutexLocker lock(&mMutex);

   auto pr = mGitServerCache->getPullRequest(mIssueNumber);

   mFrameLinks.clear();
   mComments.clear();

   const auto originalPr = pr;

   QMultiMap<QDateTime, QLayout *> bubblesMap;

   for (const auto &comment : qAsConst(pr.comments))
   {
      const auto layout = createBubbleForComment(comment);
      bubblesMap.insert(comment.creation, layout);
   }

   for (const auto &review : qAsConst(pr.reviews))
   {
      const auto layouts = new QVBoxLayout();

      const auto reviewLayout = createBubbleForReview(review);

      if (reviewLayout)
         layouts->addLayout(reviewLayout);

      auto codeReviewsLayouts = createBubbleForCodeReview(review.id, pr.reviewComment);

      for (auto layout : codeReviewsLayouts)
         layouts->addLayout(layout);

      bubblesMap.insert(review.creation, layouts);
   }

   delete mCommentsFrame;
   mCommentsFrame = new QFrame();

   const auto reviewsLayout = new QVBoxLayout(mCommentsFrame);
   reviewsLayout->setContentsMargins(QMargins());
   reviewsLayout->setSpacing(30);

   for (auto layout : bubblesMap)
   {
      if (layout)
         reviewsLayout->addLayout(layout);
   }

   reviewsLayout->addStretch();
   reviewsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

   emit frameReviewLink(originalPr, mFrameLinks);

   mIssuesLayout->addWidget(mCommentsFrame);
}

QLayout *PrCommentsList::createBubbleForComment(const Comment &comment)
{
   const auto creationLayout = new QHBoxLayout();
   creationLayout->setContentsMargins(QMargins());
   creationLayout->setSpacing(0);
   creationLayout->addWidget(new QLabel(tr("Comment by ")));

   const auto creator = new QLabel(QString("<b>%1</b>").arg(comment.creator.name));
   creator->setObjectName("CreatorLink");

   creationLayout->addWidget(creator);
   creationLayout->addWidget(createHeadline(comment.creation));
   creationLayout->addStretch();
   creationLayout->addWidget(new QLabel(comment.association));

   GitQlientSettings settings("");
   const auto colorSchema = settings.globalValue("colorSchema", "dark").toString();
   const auto style = colorSchema == "dark" ? QString::fromUtf8("dark") : QString::fromUtf8("bright");

   const auto doc = new Document(this);
   m_commentContents.append(doc);

//   const auto channel = new QWebChannel(this);
//   channel->registerObject(QStringLiteral("content"), doc);

//   const auto page = new PreviewPage(this);
//   page->setWebChannel(channel);

//   QPointer<QWebEngineView> body = new QWebEngineView();

//   connect(page, &PreviewPage::contentsSizeChanged, this, [body](const QSizeF size) {
//      if (body)
//         body->setFixedHeight(size.height());
//   });

//   body->setPage(page);
//   body->setUrl(QUrl(QString("qrc:/resources/index_%1.html").arg(style)));
//   body->setFixedHeight(20);

   doc->setText(comment.body.trimmed());

   const auto frame = new QFrame();
   frame->setObjectName("IssueIntro");

   const auto innerLayout = new QVBoxLayout(frame);
   innerLayout->setContentsMargins(10, 10, 10, 10);
   innerLayout->setSpacing(5);
   innerLayout->addLayout(creationLayout);
   innerLayout->addSpacing(20);
//   innerLayout->addWidget(body);
   innerLayout->addStretch();

   const auto layout = new QHBoxLayout();
   layout->setContentsMargins(QMargins());
   layout->setSpacing(30);
   layout->addSpacing(30);
   layout->addWidget(createAvatar(comment.creator.name, comment.creator.avatar));
   layout->addWidget(frame);

   return layout;
}

QLayout *PrCommentsList::createBubbleForReview(const Review &review)
{
   const auto frame = new QFrame();
   QString header;
   QLabel *label = nullptr;

   if (review.state == QString::fromUtf8("CHANGES_REQUESTED"))
   {
      frame->setObjectName("IssueIntroChangesRequested");

      header = tr("<b>%1</b> (%2) requested changes to the PR ").arg(review.creator.name, review.association.toLower());
   }
   else if (review.state == QString::fromUtf8("APPROVED"))
   {
      frame->setObjectName("IssueIntroApproved");

      header = tr("<b>%1</b> (%2) approved the PR ").arg(review.creator.name, review.association.toLower());
   }
   else if (review.state == QString::fromUtf8("COMMENTED"))
   {
      if (review.body.isEmpty())
      {
         delete frame;
         return nullptr;
      }

      frame->setObjectName("IssueIntroCommented");

      header = tr("<b>%1</b> (%2) reviewed the PR and added some comments ")
                   .arg(review.creator.name, review.association.toLower());

      label = createHeadline(review.creation, header);
      label->setText(label->text().append(" <p>%1</p>").arg(review.body));
   }

   if (!label)
      label = createHeadline(review.creation, header);

   const auto creationLayout = new QHBoxLayout();
   creationLayout->setContentsMargins(QMargins());
   creationLayout->setSpacing(0);
   creationLayout->addWidget(label);
   creationLayout->addStretch();

   const auto innerLayout = new QVBoxLayout(frame);
   innerLayout->setContentsMargins(10, 10, 10, 10);
   innerLayout->setSpacing(20);
   innerLayout->addLayout(creationLayout);

   const auto layout = new QHBoxLayout();
   layout->setContentsMargins(QMargins());
   layout->setSpacing(30);
   layout->addSpacing(30);
   layout->addWidget(createAvatar(review.creator.name, review.creator.avatar));
   layout->addWidget(frame);

   return layout;
}

QVector<QLayout *> PrCommentsList::createBubbleForCodeReview(int reviewId, QVector<CodeReview> &comments)
{
   QMap<int, QVector<CodeReview>> reviews;
   QVector<int> codeReviewIds;
   QVector<QLayout *> listOfCodeReviews;

   auto iter = comments.begin();

   while (iter != comments.end())
   {
      if (iter->reviewId == reviewId)
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

         const auto review = codeReviews.constFirst();

         const auto creationLayout = new QHBoxLayout();
         creationLayout->setContentsMargins(QMargins());
         creationLayout->setSpacing(0);

         const auto header
             = QString("<b>%1</b> (%2) started a review ").arg(review.creator.name, review.association.toLower());

         creationLayout->addWidget(createHeadline(review.creation, header));
         creationLayout->addStretch();

         const auto codeReviewFrame = new QFrame();
         const auto frame = new HighlightningFrame();

         const auto innerLayout = new QVBoxLayout(frame);
         innerLayout->setContentsMargins(10, 10, 10, 10);
         innerLayout->setSpacing(20);
         innerLayout->addLayout(creationLayout);

         const auto codeReviewLayout = new QVBoxLayout(codeReviewFrame);

         innerLayout->addWidget(codeReviewFrame);

         const auto code = new SourceCodeReview(review.diff.file, review.diff.diff, review.diff.line);

         codeReviewLayout->addWidget(code);
         codeReviewLayout->addSpacing(20);

         const auto commentsLayout = new QVBoxLayout();
         commentsLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
         commentsLayout->setContentsMargins(QMargins());
         commentsLayout->setSpacing(20);

         for (auto &comment : codeReviews)
            commentsLayout->addWidget(new CodeReviewComment(comment));

         codeReviewLayout->addLayout(commentsLayout);

         if (review.outdated)
         {
            const auto outdatedLabel = new ButtonLink(tr("Outdated"));
            outdatedLabel->setObjectName("OutdatedLabel");
            creationLayout->addWidget(outdatedLabel);

            codeReviewFrame->setVisible(false);

            connect(outdatedLabel, &ButtonLink::clicked, this,
                    [codeReviewFrame]() { codeReviewFrame->setVisible(!codeReviewFrame->isVisible()); });
         }
         else
         {
            const auto addComment = new QPushButton();
            addComment->setCheckable(true);
            addComment->setChecked(false);
            addComment->setIcon(QIcon(":/icons/add_comment"));
            addComment->setToolTip(tr("Add new comment"));

            creationLayout->addWidget(addComment);

            const auto inputTextEdit = new QTextEdit();
            inputTextEdit->setPlaceholderText(tr("Add your comment..."));
            inputTextEdit->setObjectName("AddReviewInput");

            const auto cancel = new QPushButton(tr("Cancel"));
            const auto add = new QPushButton(tr("Comment"));
            const auto btnsLayout = new QHBoxLayout();
            btnsLayout->setContentsMargins(QMargins());
            btnsLayout->setSpacing(0);
            btnsLayout->addWidget(cancel);
            btnsLayout->addStretch();
            btnsLayout->addWidget(add);

            const auto inputFrame = new QFrame();
            inputFrame->setVisible(false);
            const auto inputLayout = new QVBoxLayout(inputFrame);
            inputLayout->setContentsMargins(QMargins());
            inputLayout->addSpacing(20);
            inputLayout->setSpacing(10);
            inputLayout->addWidget(inputTextEdit);
            inputLayout->addLayout(btnsLayout);

            codeReviewLayout->addWidget(inputFrame);

            connect(add, &QPushButton::clicked, this, [this, inputTextEdit, commentId = review.id]() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
               addReplyToCodeReview(commentId, inputTextEdit->toMarkdown().trimmed());
#else
               addReplyToCodeReview(commentId, inputTextEdit->toPlainText().trimmed());
#endif
            });
            connect(cancel, &QPushButton::clicked, this, [inputTextEdit, addComment]() {
               inputTextEdit->clear();
               addComment->toggle();
            });
            connect(addComment, &QPushButton::toggled, this, [inputFrame, inputTextEdit](bool checked) {
               inputFrame->setVisible(checked);
               inputTextEdit->setFocus();
            });
         }

         mFrameLinks.insert(mCommentId, review.id);
         mComments.insert(mCommentId, frame);

         ++mCommentId;

         const auto layout = new QHBoxLayout();
         layout->setContentsMargins(QMargins());
         layout->setSpacing(30);
         layout->addSpacing(30);
         layout->addWidget(createAvatar(review.creator.name, review.creator.avatar));
         layout->addWidget(frame);

         listOfCodeReviews.append(layout);
      }
   }

   return listOfCodeReviews;
}

void PrCommentsList::addReplyToCodeReview(int commentId, const QString &message)
{
   mGitServerCache->getApi()->replyCodeReview(mIssueNumber, commentId, message);
}

HighlightningFrame::HighlightningFrame(QWidget *parent)
   : QFrame(parent)
{
   setObjectName("IssueIntro");
}

void HighlightningFrame::setColor(QColor color)
{
   setStyleSheet(QString("#IssueIntro { background-color: rgb(%1, %2, %3); }")
                     .arg(color.red())
                     .arg(color.green())
                     .arg(color.blue()));
}

QColor HighlightningFrame::color()
{
   return Qt::black; // getter is not really needed for now
}
