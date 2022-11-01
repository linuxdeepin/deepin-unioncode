#include "IFetcher.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>

#include <QLogger.h>

#include <QUrlQuery>

using namespace QLogger;

namespace Jenkins
{
IFetcher::IFetcher(const Config &config, QObject *parent)
   : QObject(parent)
   , mConfig(config)
{
}

IFetcher::~IFetcher()
{
   QLog_Debug("Jenkins", "Destroying repo fetcher object.");
}

void IFetcher::get(const QString &urlStr, bool customUrl)
{
   const auto apiUrl = urlStr.endsWith("api/json") || customUrl ? urlStr : urlStr + "api/json";

   QUrl url(apiUrl);

   if (!customUrl)
   {
      QUrlQuery query;
      query.addQueryItem("tree", "views[*[*]]");
      url.setQuery(query);
   }

   QNetworkRequest request(url);

   if (!mConfig.user.isEmpty() && !mConfig.token.isEmpty())
   {
      const auto data = QString("%1:%2").arg(mConfig.user, mConfig.token).toLocal8Bit().toBase64();
      request.setRawHeader("Authorization", QString(QString::fromUtf8("Basic ") + data).toLocal8Bit());
   }

   const auto reply = mConfig.accessManager->get(request);
   connect(reply, &QNetworkReply::finished, this, &IFetcher::processReply);
}

void IFetcher::processReply()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   const auto data = reply->readAll();

   if (data.isEmpty())
      QLog_Warning("Jenkins", QString("Reply from {%1} is empty.").arg(reply->url().toString()));

   const auto json = QJsonDocument::fromJson(data);

   if (json.isNull())
   {
      QLog_Error("Jenkins", QString("Data from {%1} is not a valid JSON").arg(reply->url().toString()));
      QLog_Trace("Jenkins", QString("Data received:\n%1").arg(QString::fromUtf8(data)));
      return;
   }

   processData(json);
}
}
