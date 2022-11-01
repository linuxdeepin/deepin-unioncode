#include "StageFetcher.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Jenkins
{
StageFetcher::StageFetcher(const Config &config, const JenkinsJobBuildInfo &build, QObject *parent)
   : IFetcher(config, parent)
   , mBuild(build)
{
}

void StageFetcher::triggerFetch()
{
   get(mBuild.url.append("wfapi/describe"), true);
}

void StageFetcher::processData(const QJsonDocument &json)
{
   QVector<JenkinsStageInfo> stagesVector;
   auto jsonObject = json.object();

   if (jsonObject.contains(QStringLiteral("stages")))
   {
      const auto stages = jsonObject[QStringLiteral("stages")].toArray();

      for (const auto &stageInfo : stages)
      {
         JenkinsStageInfo stage;
         const auto obj = stageInfo.toObject();

         if (obj.contains(QStringLiteral("name")) && !obj[QStringLiteral("name")].toString().contains("Build"))
         {
            stage.name = obj[QStringLiteral("name")].toString();

            if (obj.contains(QStringLiteral("id")))
               stage.id = obj[QStringLiteral("id")].toInt();
            if (obj.contains(QStringLiteral("links")))
               stage.url = obj[QStringLiteral("links")].toObject()["self"].toObject()["href"].toString();
            if (obj.contains(QStringLiteral("status")))
               stage.result = obj[QStringLiteral("status")].toString();
            if (obj.contains(QStringLiteral("durationMillis")))
               stage.duration = obj[QStringLiteral("durationMillis")].toInt();

            stagesVector.append(stage);
         }
      }
   }

   emit signalStagesReceived(stagesVector);
}
}
