#pragma once

#include <JenkinsJobInfo.h>
#include <IFetcher.h>

#include <QFrame>

class CheckBox;
class QVBoxLayout;
class QHBoxLayout;
class QButtonGroup;
class QRadioButton;
class QPinnableTabWidget;
class QNetworkAccessManager;
class QPlainTextEdit;
class QPushButton;
class ButtonLink;

namespace Jenkins
{

class JenkinsJobPanel : public QFrame
{
   Q_OBJECT

signals:
   void gotoPullRequest(int prNumber);
   void gotoBranch(const QString &branchName);

public:
   explicit JenkinsJobPanel(const IFetcher::Config &config, QWidget *parent = nullptr);

   void loadJobInfo(const JenkinsJobInfo &job);

private:
   IFetcher::Config mConfig;
   ButtonLink *mName = nullptr;
   ButtonLink *mUrl = nullptr;
   QPushButton *mBuild = nullptr;
   QFrame *mScrollFrame = nullptr;
   QVBoxLayout *mBuildListLayout = nullptr;
   QHBoxLayout *mLastBuildLayout = nullptr;
   QFrame *mLastBuildFrame = nullptr;
   QPinnableTabWidget *mTabWidget = nullptr;
   JenkinsJobInfo mRequestedJob;
   int mTmpBuildsCounter = 0;
   QVector<QWidget *> mTempWidgets;
   QVector<QString> mDownloadedFiles;
   QNetworkAccessManager *mManager = nullptr;
   QMap<int, int> mTabBuildMap;
   QMap<QString, QPair<JobConfigFieldType, QVariant>> mBuildValues;

   void appendJobsData(const QString &jobName, const JenkinsJobBuildInfo &build);
   void fillBuildLayout(const Jenkins::JenkinsJobBuildInfo &build, QHBoxLayout *layout);
   void requestFile(const Jenkins::JenkinsJobBuildInfo &build);
   void storeFile(int buildNumber);
   void createBuildConfigPanel();
   void triggerBuild();
   void showArtifacts(const Jenkins::JenkinsJobBuildInfo &build);
   void downloadArtifact(const JenkinsJobBuildInfo::Artifact &artifact, int number);
   void storeArtifact(const QString &fileName, int buildNumber);
   void reloadJobInfo();
};
}
