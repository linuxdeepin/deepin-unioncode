#pragma once

#include <IFetcher.h>
#include <JenkinsJobInfo.h>

namespace Jenkins
{

class StageFetcher : public IFetcher
{
   Q_OBJECT

signals:
   void signalStagesReceived(const QVector<JenkinsStageInfo> stages);

public:
   StageFetcher(const IFetcher::Config &config, const JenkinsJobBuildInfo &build, QObject *parent = nullptr);

   void triggerFetch() override;

private:
   JenkinsJobBuildInfo mBuild;

   void processData(const QJsonDocument &json) override;
};

}
