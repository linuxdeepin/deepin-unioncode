#pragma once

#include <QWidget>
#include <QMap>

#include <JenkinsViewInfo.h>
#include <IFetcher.h>

class GitBase;
class QStackedLayout;
class QButtonGroup;
class QHBoxLayout;
class QVBoxLayout;
class QTimer;

namespace Jenkins
{

class RepoFetcher;
class JobContainer;

class JenkinsWidget : public QWidget
{
   Q_OBJECT

signals:
   void gotoPullRequest(int prNumber);
   void gotoBranch(const QString &branchName);

public:
   JenkinsWidget(const QSharedPointer<GitBase> &git, QWidget *parent = nullptr);
   ~JenkinsWidget() override;

   void reload() const;

private:
   QSharedPointer<GitBase> mGit;
   IFetcher::Config mConfig;
   QStackedLayout *mStackedLayout = nullptr;
   RepoFetcher *mRepoFetcher = nullptr;
   QHBoxLayout *mBodyLayout = nullptr;
   QButtonGroup *mBtnGroup = nullptr;
   QVBoxLayout *mButtonsLayout = nullptr;
   QVector<JenkinsViewInfo> mViews;
   QMap<QString, JobContainer *> mJobsMap;
   QTimer *mTimer = nullptr;

   void configureGeneralView(const QVector<JenkinsViewInfo> &views);
};

}
