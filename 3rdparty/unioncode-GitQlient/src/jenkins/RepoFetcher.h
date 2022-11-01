#pragma once

#include <IFetcher.h>
#include <JenkinsViewInfo.h>

namespace Jenkins
{

class RepoFetcher final : public IFetcher
{
   Q_OBJECT

signals:
   void signalViewsReceived(const QVector<JenkinsViewInfo> &views);

public:
   explicit RepoFetcher(const IFetcher::Config &config, const QString &url, QObject *parent = nullptr);
   ~RepoFetcher();

   void triggerFetch() override;

private:
   QString mUrl;
   void processData(const QJsonDocument &json) override;
};

}
