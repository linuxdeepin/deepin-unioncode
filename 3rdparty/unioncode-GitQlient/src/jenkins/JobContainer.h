#pragma once

#include <JenkinsViewInfo.h>
#include <JenkinsJobInfo.h>
#include <IFetcher.h>

#include <QFrame>

class QVBoxLayout;
class QTreeWidgetItem;
class QListWidget;
class QLabel;
class QHBoxLayout;
class QTreeWidget;

namespace Jenkins
{
class JenkinsJobPanel;
class JobFetcher;

class JobContainer : public QFrame
{
   Q_OBJECT

signals:
   void signalJobAreViews(const QVector<JenkinsViewInfo> &views);
   void gotoPullRequest(int prNumber);
   void gotoBranch(const QString &branchName);

public:
   explicit JobContainer(const IFetcher::Config &config, const JenkinsViewInfo &viewInfo, QWidget *parent = nullptr);

   void reload();

private:
   IFetcher::Config mConfig;
   JenkinsViewInfo mView;
   JobFetcher *mJobFetcher = nullptr;
   QHBoxLayout *mMainLayout = nullptr;
   QVBoxLayout *mJobListLayout = nullptr;
   JenkinsJobPanel *mJobPanel = nullptr;
   QVector<JenkinsJobInfo> mJobsList;
   QTreeWidget *mJobsTree = nullptr;
   QMap<QString, QListWidget *> mListsMap;

   void addJobs(const QMultiMap<QString, JenkinsJobInfo> &jobs);
   void requestUpdatedJobInfo(const JenkinsJobInfo &jobInfo);
   void onJobInfoReceived(JenkinsJobInfo oldInfo, const JenkinsJobInfo &newInfo);
   void showJobInfo(QTreeWidgetItem *item, int column);
   QIcon getIconForJob(JenkinsJobInfo job) const;
   void createHeader(const QString &name, QListWidget *listWidget);
   void onHeaderClicked(QListWidget *listWidget, QLabel *mTagArrow);
};
}
