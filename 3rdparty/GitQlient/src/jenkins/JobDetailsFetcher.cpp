#include "JobDetailsFetcher.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Jenkins
{

JobDetailsFetcher::JobDetailsFetcher(const IFetcher::Config &config, const JenkinsJobInfo &info, QObject *parent)
   : IFetcher(config, parent)
   , mInfo(info)
{
}

void JobDetailsFetcher::triggerFetch()
{
   get(mInfo.url + QString::fromUtf8("api/json"), true);
}

void JobDetailsFetcher::processData(const QJsonDocument &json)
{
   auto jsonObject = json.object();

   readHealthReportsPartFor(jsonObject);
   readBuildsListFor(jsonObject);
   retrieveBuildConfig(jsonObject[QStringLiteral("property")].toArray());
   readBuildableFlagFor(jsonObject);
   readIsQueuedFlagFor(jsonObject);

   emit signalJobDetailsRecieved(mInfo);
}

void JobDetailsFetcher::readHealthReportsPartFor(QJsonObject &jsonObject)
{
   if (jsonObject.contains(QStringLiteral("healthReport")))
   {
      const auto healthArray = jsonObject[QStringLiteral("healthReport")].toArray();
      for (const auto &item : healthArray)
      {
         JenkinsJobInfo::HealthStatus status;
         QJsonObject healthObject = item.toObject();
         if (healthObject.contains(QStringLiteral("score")))
            status.score = healthObject[QStringLiteral("score")].toInt();
         if (healthObject.contains(QStringLiteral("description")))
            status.description = healthObject[QStringLiteral("description")].toString();
         if (healthObject.contains(QStringLiteral("iconClassName")))
            status.iconClassName = healthObject[QStringLiteral("iconClassName")].toString();

         mInfo.healthStatus = status;
      }
   }
}

void JobDetailsFetcher::readBuildsListFor(QJsonObject &jsonObject)
{
   if (jsonObject.contains(QStringLiteral("builds")))
   {
      const auto buildsArray = jsonObject[QStringLiteral("builds")].toArray();

      for (const auto &build : buildsArray)
      {
         const auto buildObject = build.toObject();
         if (buildObject.contains(QStringLiteral("number")) && buildObject.contains(QStringLiteral("url")))
         {
            JenkinsJobBuildInfo build;
            build.number = buildObject[QStringLiteral("number")].toInt();
            build.url = buildObject[QStringLiteral("url")].toString();

            mInfo.builds.append(build);
         }
      }
   }
}

void JobDetailsFetcher::retrieveBuildConfig(const QJsonArray &propertyArray)
{
   QVector<JenkinsJobBuildConfig> configParams;

   for (const auto &property : propertyArray)
   {
      auto propertyObj = property.toObject();

      if (propertyObj[QStringLiteral("_class")].toString().contains("ParametersDefinitionProperty"))
      {
         const auto params = propertyObj[QStringLiteral("parameterDefinitions")].toArray();

         for (const auto &config : params)
         {
            JenkinsJobBuildConfig jobConfig;
            jobConfig.name = config[QStringLiteral("name")].toString();

            if (config[QStringLiteral("type")].toString() == "BooleanParameterDefinition")
               jobConfig.fieldType = JobConfigFieldType::Bool;
            else if (config[QStringLiteral("type")].toString() == "StringParameterDefinition")
               jobConfig.fieldType = JobConfigFieldType::String;
            else if (config[QStringLiteral("type")].toString() == "ChoiceParameterDefinition")
            {
               jobConfig.fieldType = JobConfigFieldType::Choice;

               const auto choices = config[QStringLiteral("choices")].toArray();

               for (const auto &choiceValue : choices)
                  jobConfig.choicesValues.append(choiceValue.toString());
            }

            jobConfig.defaultValue
                = config[QStringLiteral("defaultParameterValue")].toObject()[QStringLiteral("value")].toVariant();

            configParams.append(jobConfig);
         }
      }
   }

   mInfo.configFields = configParams;
}

void JobDetailsFetcher::readBuildableFlagFor(QJsonObject &)
{
   /*
   if (jsonObject.contains(QStringLiteral("buildable")))
      job.setIsBuildable(jsonObject[QStringLiteral("buildable")].toBool());
   else
      job.setIsBuildable(true);
*/
}

void JobDetailsFetcher::readIsQueuedFlagFor(QJsonObject &)
{
   /*
    if (jsonObject.contains(QStringLiteral("inQueue")))
       job.setIsQueued(jsonObject[QStringLiteral("inQueue")].toBool());
    else
       job.setIsQueued(false);
*/
}

}
