#pragma once

#include <IFetcher.h>
#include <JenkinsJobInfo.h>

#include <QMutex>

namespace Jenkins
{

class JobDetailsFetcher : public IFetcher
{
   Q_OBJECT

signals:
   void signalJobDetailsRecieved(const JenkinsJobInfo &updatedInfo);

public:
   JobDetailsFetcher(const Config &config, const JenkinsJobInfo &info, QObject *parent = nullptr);

   void triggerFetch() override;

private:
   JenkinsJobInfo mInfo;
   QVector<JenkinsJobBuildInfo> mBuildsInfo;
   QMap<int, JenkinsJobBuildInfo> mTmpBuilds;
   QMutex mMutex;

   void processData(const QJsonDocument &json) override;
   void readHealthReportsPartFor(QJsonObject &jsonObject);
   void readBuildsListFor(QJsonObject &jsonObject);
   void retrieveBuildConfig(const QJsonArray &propertyArray);
   void readBuildableFlagFor(QJsonObject &jsonObject);
   void readIsQueuedFlagFor(QJsonObject &jsonObject);
};

}
