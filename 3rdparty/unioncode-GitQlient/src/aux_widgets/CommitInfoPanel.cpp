#include <CommitInfoPanel.h>

#include <ButtonLink.hpp>
#include <CommitInfo.h>

#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QScrollArea>
#include <QToolTip>
#include <QVBoxLayout>

CommitInfoPanel::CommitInfoPanel(QWidget *parent)
   : QFrame(parent)
   , mLabelSha(new ButtonLink())
   , mLabelTitle(new QLabel())
   , mLabelDescription(new QLabel())
   , mLabelAuthor(new QLabel())
   , mLabelDateTime(new QLabel())
{
   mLabelSha->setObjectName("labelSha");
   mLabelSha->setAlignment(Qt::AlignCenter);
   mLabelSha->setWordWrap(true);

   QFont font1;
   font1.setBold(true);
   font1.setWeight(QFont::Bold);
   mLabelTitle->setFont(font1);
   mLabelTitle->setAlignment(Qt::AlignCenter);
   mLabelTitle->setWordWrap(true);
   mLabelTitle->setObjectName("labelTitle");

   mLabelDescription->setWordWrap(true);
   mLabelDescription->setObjectName("labelDescription");

   mScrollArea = new QScrollArea();
   mScrollArea->setWidget(mLabelDescription);
   mScrollArea->setWidgetResizable(true);
   mScrollArea->setFixedHeight(50);

   mLabelAuthor->setObjectName("labelAuthor");

   mLabelDateTime->setObjectName("labelDateTime");

   const auto wipSeparator = new QFrame();
   wipSeparator->setObjectName("separator");

   const auto descriptionLayout = new QVBoxLayout(this);
   descriptionLayout->setContentsMargins(0, 0, 0, 0);
   descriptionLayout->setSpacing(0);
   descriptionLayout->addWidget(mLabelSha);
   descriptionLayout->addWidget(mLabelTitle);
   descriptionLayout->addWidget(mScrollArea);
   descriptionLayout->addWidget(wipSeparator);
   descriptionLayout->addWidget(mLabelAuthor);
   descriptionLayout->addWidget(mLabelDateTime);

   connect(mLabelSha, &ButtonLink::clicked, this, [this]() {
      const auto button = qobject_cast<ButtonLink *>(sender());
      QApplication::clipboard()->setText(button->data().toString());
      QToolTip::showText(QCursor::pos(), tr("Copied!"), button);
   });
}

void CommitInfoPanel::configure(const CommitInfo &commit)
{
   mLabelSha->setText(commit.sha.left(8));
   mLabelSha->setData(commit.sha);
   mLabelSha->setToolTip("Click to save");

   const auto authorName = commit.committer.split("<").first();
   mLabelTitle->setText(commit.shortLog);
   mLabelAuthor->setText(authorName);

   QDateTime commitDate = QDateTime::fromSecsSinceEpoch(commit.dateSinceEpoch.count());
   mLabelDateTime->setText(commitDate.toString("dd/MM/yyyy hh:mm"));

   const auto description = commit.longLog;
   mLabelDescription->setText(description.isEmpty() ? "<No description provided>" : description);

   QFontMetrics fm(mLabelDescription->font());
   const auto neededsize = fm.boundingRect(QRect(0, 0, 300, 250), Qt::TextWordWrap, mLabelDescription->text());
   auto height = neededsize.height();

   if (height > 250)
      height = 250;
   else if (height < 50)
      height = 50;

   mScrollArea->setFixedHeight(height);

   auto f = mLabelDescription->font();
   f.setItalic(description.isEmpty());
   mLabelDescription->setFont(f);
}

void CommitInfoPanel::clear()
{
   mLabelSha->clear();
   mLabelTitle->clear();
   mLabelAuthor->clear();
   mLabelDateTime->clear();
   mLabelDescription->clear();
}
