#pragma once

#include <IFetcher.h>
#include <JenkinsJobInfo.h>

namespace Jenkins
{

class JobFetcher final : public IFetcher
{
   Q_OBJECT

signals:
   void signalJobsReceived(const QMultiMap<QString, JenkinsJobInfo> &jobs);

public:
   explicit JobFetcher(const IFetcher::Config &config, const QString &jobUrl, QObject *parent = nullptr);

   void triggerFetch() override;

private:
   QString mJobUrl;

   void processData(const QJsonDocument &json) override;
};
}
