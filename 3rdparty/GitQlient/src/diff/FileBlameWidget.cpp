#include "FileBlameWidget.h"

#include <ButtonLink.hpp>
#include <CommitInfo.h>
#include <GitCache.h>
#include <GitHistory.h>

#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QtMath>

#include <array>

namespace
{
static const int kTotalColors = 8;
static const std::array<const char *, kTotalColors> kBorderColors { { "25, 65, 99", "36, 95, 146", "44, 116, 177",
                                                                      "56, 136, 205", "87, 155, 213", "118, 174, 221",
                                                                      "150, 192, 221", "197, 220, 240" } };
qint64 kSecondsNewest = 0;
qint64 kSecondsOldest = QDateTime::currentDateTime().toSecsSinceEpoch();
qint64 kIncrementSecs = 0;
}

FileBlameWidget::FileBlameWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                                 QWidget *parent)
   : QFrame(parent)
   , mCache(cache)
   , mGit(git)
   , mAnotation(new QFrame())
   , mCurrentSha(new QLabel())
   , mPreviousSha(new QLabel())
{
   setAttribute(Qt::WA_DeleteOnClose);

   mAnotation->setObjectName("AnnotationFrame");

   auto initialLayout = new QGridLayout(mAnotation);
   initialLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);
   initialLayout->addWidget(new QLabel(tr("Select a file to blame")), 1, 1);
   initialLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 2);

   mInfoFont.setPointSize(9);

   mCodeFont = QFont(mInfoFont);
   mCodeFont.setFamily("DejaVu Sans Mono");
   mCodeFont.setPointSize(8);

   mScrollArea = new QScrollArea();
   mScrollArea->setWidget(mAnotation);
   mScrollArea->setWidgetResizable(true);

   const auto lSha = new QLabel(tr("Current SHA:"));
   const auto lSha2 = new QLabel(tr("Previous SHA:"));

   const auto separator = new QFrame();
   separator->setObjectName("separator");

   const auto shasLayout = new QGridLayout();
   shasLayout->setSpacing(10);
   shasLayout->setContentsMargins(QMargins());
   shasLayout->addWidget(lSha, 0, 0);
   shasLayout->addWidget(mCurrentSha, 0, 1);
   shasLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 2);
   shasLayout->addWidget(lSha2, 1, 0);
   shasLayout->addWidget(mPreviousSha, 1, 1);
   shasLayout->addWidget(separator, 2, 0, 1, 3);

   const auto layout = new QVBoxLayout(this);
   layout->setContentsMargins(10, 10, 10, 0);
   layout->setSpacing(0);
   layout->addLayout(shasLayout);
   layout->addWidget(mScrollArea);
}

void FileBlameWidget::setup(const QString &fileName, const QString &currentSha, const QString &previousSha)
{
   mCurrentFile = fileName;
   QScopedPointer<GitHistory> git(new GitHistory(mGit));
   const auto ret = git->blame(mCurrentFile, currentSha);

   if (ret.success && !ret.output.startsWith("fatal:"))
   {
      delete mAnotation;
      mAnotation = nullptr;

      mCurrentSha->setText(currentSha);
      mPreviousSha->setText(previousSha);

      const auto annotations = processBlame(ret.output);
      formatAnnotatedFile(annotations);
   }
   else
      QMessageBox::warning(
          this, tr("File not in Git"),
          tr("The file {%1} is not under Git control version. You cannot blame it.").arg(mCurrentFile));
}

void FileBlameWidget::reload(const QString &currentSha, const QString &previousSha)
{
   setup(mCurrentFile, currentSha, previousSha);
}

QString FileBlameWidget::getCurrentSha() const
{
   return mCurrentSha->text();
}

QVector<FileBlameWidget::Annotation> FileBlameWidget::processBlame(const QString &blame)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
   const auto lines = blame.split("\n", Qt::SkipEmptyParts);
#else
   const auto lines = blame.split("\n", QString::SkipEmptyParts);
#endif
   QVector<Annotation> annotations;

   for (const auto &line : lines)
   {
      auto start = 0;
      auto indexOfTab = line.indexOf('\t');
      const auto shortSha = line.mid(start, indexOfTab);
      const auto revision = mCache->commitInfo(shortSha);

      start = indexOfTab + 1;
      indexOfTab = line.indexOf('\t', start);
      const auto name = line.mid(start, indexOfTab - start).remove("(");

      start = indexOfTab + 1;
      indexOfTab = line.indexOf('\t', start);
      const auto dtValue = line.mid(start, indexOfTab - start);
      const auto dt = QDateTime::fromString(dtValue, Qt::ISODate);

      start = indexOfTab + 1;

      const auto lineNumAndContent = line.mid(start);
      const auto divisorChar = lineNumAndContent.indexOf(")");
      const auto lineText = lineNumAndContent.mid(0, divisorChar);
      const auto content = lineNumAndContent.mid(divisorChar + 1, lineNumAndContent.count() - lineText.count() - 1);

      annotations.append({ revision.sha, name, dt, lineText.toInt(), content });

      if (revision.sha != CommitInfo::ZERO_SHA)
      {
         const auto dtSinceEpoch = dt.toSecsSinceEpoch();

         if (kSecondsNewest < dtSinceEpoch)
            kSecondsNewest = dtSinceEpoch;

         if (kSecondsOldest > dtSinceEpoch)
            kSecondsOldest = dtSinceEpoch;
      }
   }

   kIncrementSecs = kSecondsNewest != kSecondsOldest ? (kSecondsNewest - kSecondsOldest) / (kTotalColors - 1) : 1;

   return annotations;
}

void FileBlameWidget::formatAnnotatedFile(const QVector<Annotation> &annotations)
{
   auto labelRow = 0;
   auto labelRowSpan = 1;
   QLabel *dateLabel = nullptr;
   QLabel *authorLabel = nullptr;
   ButtonLink *messageLabel = nullptr;

   const auto annotationLayout = new QGridLayout();
   annotationLayout->setContentsMargins(QMargins());
   annotationLayout->setSpacing(0);

   const auto totalAnnot = annotations.count();
   for (auto row = 0; row < totalAnnot; ++row)
   {
      const auto &lastAnnotation = row == 0 ? Annotation() : annotations.at(row - 1);

      if (lastAnnotation.sha != annotations.at(row).sha)
      {
         if (dateLabel)
            annotationLayout->addWidget(dateLabel, labelRow, 0);

         if (authorLabel)
            annotationLayout->addWidget(authorLabel, labelRow, 1);

         if (messageLabel)
            annotationLayout->addWidget(messageLabel, labelRow, 2);

         dateLabel = createDateLabel(annotations.at(row), row == 0);
         authorLabel = createAuthorLabel(annotations.at(row).author, row == 0);
         messageLabel = createMessageLabel(annotations.at(row).sha, row == 0);

         labelRow = row;
         labelRowSpan = 1;
      }
      else
         ++labelRowSpan;

      annotationLayout->addWidget(createNumLabel(annotations.at(row), row), row, 3);
      annotationLayout->addWidget(createCodeLabel(annotations.at(row).content), row, 4);
   }

   // Adding the last row
   if (dateLabel)
      annotationLayout->addWidget(dateLabel, labelRow, 0);

   if (authorLabel)
      annotationLayout->addWidget(authorLabel, labelRow, 1);

   if (messageLabel)
      annotationLayout->addWidget(messageLabel, labelRow, 2);

   annotationLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), totalAnnot, 4);

   mAnotation = new QFrame();
   mAnotation->setObjectName("AnnotationFrame");
   mAnotation->setLayout(annotationLayout);

   mScrollArea->setWidget(mAnotation);
   mScrollArea->setWidgetResizable(true);
}

QLabel *FileBlameWidget::createDateLabel(const Annotation &annotation, bool isFirst)
{
   auto isWip = annotation.sha == CommitInfo::ZERO_SHA;
   QString when;

   if (!isWip)
   {
      const auto days = annotation.dateTime.daysTo(QDateTime::currentDateTime());
      const auto secs = annotation.dateTime.secsTo(QDateTime::currentDateTime());
      if (days > 365)
         when.append(tr("more than 1 year ago"));
      else if (days > 1)
         when.append(QString::number(days)).append(tr(" days ago"));
      else if (days == 1)
         when.append(tr("yesterday"));
      else if (secs > 3600)
         when.append(QString::number(secs / 3600)).append(tr(" hours ago"));
      else if (secs == 3600)
         when.append(tr("1 hour ago"));
      else if (secs > 60)
         when.append(QString::number(secs / 60)).append(tr(" minutes ago"));
      else if (secs == 60)
         when.append(tr("1 minute ago"));
      else
         when.append(QString::number(secs)).append(tr(" secs ago"));
   }

   const auto dateLabel = new QLabel(when);
   dateLabel->setObjectName(isFirst ? QString("authorPrimusInterPares") : QString("authorFirstOfItsName"));
   dateLabel->setToolTip(annotation.dateTime.toString("dd/MM/yyyy hh:mm"));
   dateLabel->setFont(mInfoFont);
   dateLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

   return dateLabel;
}

QLabel *FileBlameWidget::createAuthorLabel(const QString &author, bool isFirst)
{
   const auto authorLabel = new QLabel(author);
   authorLabel->setObjectName(isFirst ? QString("authorPrimusInterPares") : QString("authorFirstOfItsName"));
   authorLabel->setFont(mInfoFont);
   authorLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

   return authorLabel;
}

ButtonLink *FileBlameWidget::createMessageLabel(const QString &sha, bool isFirst)
{
   const auto revision = mCache->commitInfo(sha);
   auto commitMsg = tr("Local changes");

   if (!revision.sha.isEmpty())
   {
      auto log = revision.shortLog;

      if (log.count() > 47)
         log = log.left(47) + QString("...");

      commitMsg = log;
   }

   const auto messageLabel = new ButtonLink(commitMsg);
   messageLabel->setObjectName(isFirst ? QString("primusInterPares") : QString("firstOfItsName"));
   messageLabel->setToolTip(QString("<p>%1</p><p>%2</p>").arg(sha, commitMsg));
   messageLabel->setFont(mInfoFont);

   connect(messageLabel, &ButtonLink::clicked, this, [this, sha]() { emit signalCommitSelected(sha); });

   return messageLabel;
}

QLabel *FileBlameWidget::createNumLabel(const Annotation &annotation, int row)
{
   const auto numberLabel = new QLabel(QString::number(row + 1));
   numberLabel->setFont(mCodeFont);
   numberLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
   numberLabel->setObjectName("numberLabel");
   numberLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

   if (annotation.sha != CommitInfo::ZERO_SHA)
   {
      const auto dtSinceEpoch = annotation.dateTime.toSecsSinceEpoch();
      const auto colorIndex = qCeil((kSecondsNewest - dtSinceEpoch) / kIncrementSecs);
      numberLabel->setStyleSheet(
          QString("QLabel { border-left: 5px solid rgb(%1) }").arg(QString::fromUtf8(kBorderColors.at(colorIndex))));
   }
   else
      numberLabel->setStyleSheet("QLabel { border-left: 5px solid #D89000 }");

   return numberLabel;
}

QLabel *FileBlameWidget::createCodeLabel(const QString &content)
{
   const auto contentLabel = new QLabel(content);
   contentLabel->setFont(mCodeFont);
   contentLabel->setObjectName("normalLabel");

   return contentLabel;
}
