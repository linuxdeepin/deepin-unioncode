#include "JobFetcher.h"

#include <JobDetailsFetcher.h>

#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QLogger.h>

using namespace QLogger;

namespace Jenkins
{

JobFetcher::JobFetcher(const Config &config, const QString &jobUrl, QObject *parent)
   : IFetcher(config, parent)
   , mJobUrl(jobUrl)
{
}

void JobFetcher::triggerFetch()
{
   get(mJobUrl);
}

void JobFetcher::processData(const QJsonDocument &json)
{
   const auto jsonObject = json.object();
   QMultiMap<QString, JenkinsJobInfo> jobsMap;

   if (!jsonObject.contains(QStringLiteral("views")))
      QLog_Debug("Jenkins", "Views not found.");
   else if (!jsonObject.contains(QStringLiteral("jobs")))
      QLog_Debug("Jenkins", "Jobs not found.");

   const auto views = jsonObject[QStringLiteral("views")].toArray();

   for (const auto &view : views)
   {
      const auto _class = view[QStringLiteral("_class")].toString();
      const auto generalView = _class.contains("AllView");
      const auto jsonObject = view.toObject();
      const auto jobs = jsonObject[QStringLiteral("jobs")].toArray();

      for (const auto &job : jobs)
      {
         QJsonObject jobObject = job.toObject();
         QString url;

         if (jobObject.contains(QStringLiteral("url")))
            url = jobObject[QStringLiteral("url")].toString();

         if (jobObject[QStringLiteral("_class")].toString().contains("WorkflowMultiBranchProject") && !generalView)
         {
            get(url);
         }
         else if (jobObject[QStringLiteral("_class")].toString().contains("WorkflowJob"))
         {
            JenkinsJobInfo jobInfo;
            jobInfo.url = url;

            if (jobObject.contains(QStringLiteral("displayName")))
               jobInfo.name = jobObject[QStringLiteral("displayName")].toString();
            else
               jobInfo.name = jobObject[QStringLiteral("name")].toString();

            if (jobObject.contains(QStringLiteral("color")))
               jobInfo.color = jobObject[QStringLiteral("color")].toString();

            jobsMap.insert(view[QStringLiteral("name")].toString(), jobInfo);
         }
      }
   }

   emit signalJobsReceived(jobsMap);
}

}
