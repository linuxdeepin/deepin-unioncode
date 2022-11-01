#include <CommitInfo.h>
#include <CommitInfoPanel.h>
#include <CommitInfoWidget.h>
#include <FileListWidget.h>
#include <GitCache.h>

#include <QDateTime>
#include <QLabel>
#include <QVBoxLayout>

#include <QLogger.h>

using namespace QLogger;

CommitInfoWidget::CommitInfoWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                                   QWidget *parent)
   : QFrame(parent)
   , mCache(cache)
   , mGit(git)
   , mInfoPanel(new CommitInfoPanel())
   , mFileListWidget(new FileListWidget(mGit, mCache))
{
   setAttribute(Qt::WA_DeleteOnClose);

   mFileListWidget->setObjectName("fileListWidget");

   const auto wipSeparator = new QFrame();
   wipSeparator->setObjectName("separator");

   const auto mainLayout = new QGridLayout(this);
   mainLayout->setSpacing(0);
   mainLayout->setContentsMargins(0, 0, 0, 0);
   mainLayout->addWidget(mInfoPanel, 0, 0);
   mainLayout->addWidget(wipSeparator, 1, 0);
   mainLayout->addWidget(mFileListWidget, 2, 0);
   mainLayout->setRowStretch(1, 0);
   mainLayout->setRowStretch(2, 0);
   mainLayout->setRowStretch(2, 1);

   connect(mFileListWidget, &FileListWidget::itemDoubleClicked, this,
           [this](QListWidgetItem *item) { emit signalOpenFileCommit(mCurrentSha, mParentSha, item->text(), false); });
   connect(mFileListWidget, &FileListWidget::signalShowFileHistory, this, &CommitInfoWidget::signalShowFileHistory);
   connect(mFileListWidget, &FileListWidget::signalEditFile, this, &CommitInfoWidget::signalEditFile);
}

void CommitInfoWidget::configure(const QString &sha)
{
   if (sha == mCurrentSha)
      return;

   clear();

   mCurrentSha = sha;
   mParentSha = sha;

   if (sha != CommitInfo::ZERO_SHA && !sha.isEmpty())
   {
      const auto commit = mCache->commitInfo(sha);

      if (!commit.sha.isEmpty())
      {
         QLog_Info("UI", QString("Loading information of the commit {%1}").arg(sha));
         mCurrentSha = commit.sha;
         mParentSha = commit.firstParent();

         mInfoPanel->configure(commit);

         mFileListWidget->insertFiles(mCurrentSha, mParentSha);
      }
   }
}

QString CommitInfoWidget::getCurrentCommitSha() const
{
   return mCurrentSha;
}

void CommitInfoWidget::clear()
{
   mCurrentSha = QString();
   mParentSha = QString();

   mFileListWidget->clear();
}
