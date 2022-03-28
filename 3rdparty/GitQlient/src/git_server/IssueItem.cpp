#include <IssueItem.h>

#include <ButtonLink.hpp>
#include <PullRequest.h>

#include <QDir>
#include <QFile>
#include <QUrl>
#include <QGridLayout>
#include <QDesktopServices>
#include <QLocale>

using namespace GitServer;

IssueItem::IssueItem(const Issue &issueData, QWidget *parent)
   : QFrame(parent)
   , mComments(new QLabel())
{
   setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

   fillWidget(issueData);

   mComments->setText(QString::number(issueData.commentsCount));
}

IssueItem::IssueItem(const PullRequest &issueData, QWidget *parent)
   : QFrame(parent)
   , mComments(new QLabel())
{
   fillWidget(issueData);

   mComments->setText(QString::number(issueData.commentsCount + issueData.reviewCommentsCount));
}

void IssueItem::fillWidget(const Issue &issueData)
{
   const auto title = new ButtonLink(QString("#%1 - %2").arg(issueData.number).arg(issueData.title));
   title->setWordWrap(true);
   title->setObjectName("IssueTitle");
   connect(title, &ButtonLink::clicked, this, [this, issueNum = issueData.number]() { emit selected(issueNum); });

   const auto titleLayout = new QHBoxLayout();
   titleLayout->setContentsMargins(QMargins());
   titleLayout->setSpacing(0);
   titleLayout->addWidget(title);

   const auto creationLayout = new QHBoxLayout();
   creationLayout->setContentsMargins(QMargins());
   creationLayout->setSpacing(0);
   creationLayout->addWidget(new QLabel(tr("<i>Created by </i>")));
   const auto creator = new ButtonLink(QString("<i><b>%1</b></i>").arg(issueData.creator.name));
   creator->setObjectName("CreatorLink");
   connect(creator, &ButtonLink::clicked, [url = issueData.url]() { QDesktopServices::openUrl(url); });

   creationLayout->addWidget(creator);

   const auto days = issueData.creation.daysTo(QDateTime::currentDateTime());
   const auto whenText = days <= 30
       ? tr("<i> %1 days ago</i>").arg(days)
       : tr("<i> on %1</i>").arg(issueData.creation.date().toString(QLocale().dateFormat(QLocale::ShortFormat)));

   const auto whenLabel = new QLabel(whenText);
   whenLabel->setToolTip(issueData.creation.toString(QLocale().dateFormat(QLocale::ShortFormat)));

   creationLayout->addWidget(whenLabel);
   creationLayout->addStretch();

   const auto layout = new QVBoxLayout();
   layout->setContentsMargins(QMargins());
   layout->setSpacing(5);
   layout->addLayout(titleLayout);
   layout->addLayout(creationLayout);

   if (!issueData.assignees.isEmpty())
   {
      const auto assigneeLayout = new QHBoxLayout();
      assigneeLayout->setContentsMargins(QMargins());
      assigneeLayout->setSpacing(0);
      assigneeLayout->addWidget(new QLabel(tr("Assigned to ")));

      auto count = 0;
      const auto totalAssignees = issueData.assignees.count();
      for (auto &assignee : issueData.assignees)
      {
         const auto assigneLabel = new ButtonLink(QString("<b>%1</b>").arg(assignee.name));
         assigneLabel->setObjectName("CreatorLink");
         connect(assigneLabel, &ButtonLink::clicked, [url = assignee.url]() { QDesktopServices::openUrl(url); });
         assigneeLayout->addWidget(assigneLabel);

         if (count++ < totalAssignees - 1)
            assigneeLayout->addWidget(new QLabel(", "));
      }
      assigneeLayout->addStretch();

      layout->addLayout(assigneeLayout);
   }

   if (!issueData.labels.isEmpty())
   {
      const auto labelsLayout = new QHBoxLayout();
      labelsLayout->setContentsMargins(QMargins());
      labelsLayout->setSpacing(10);

      QStringList labelsList;

      for (auto &label : issueData.labels)
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
         labelsLayout->addWidget(labelWidget);
      }

      const auto milestone = new QLabel(QString("%1").arg(issueData.milestone.title));
      milestone->setObjectName("IssueLabel");
      labelsLayout->addWidget(milestone);
      labelsLayout->addStretch();
      layout->addLayout(labelsLayout);
   }

   QPixmap pic(":/icons/comment");
   pic = pic.scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation);

   const auto icon = new QLabel();
   icon->setPixmap(pic);

   const auto commentsLayout = new QGridLayout();
   commentsLayout->addWidget(mComments, 0, 0);
   commentsLayout->addWidget(icon, 0, 1);
   commentsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0, 1, 2);

   const auto mainLayout = new QHBoxLayout(this);
   mainLayout->setContentsMargins(10, 10, 10, 10);
   mainLayout->addLayout(layout);
   mainLayout->addLayout(commentsLayout);
}
