#include <IRestApi.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QLogger.h>

using namespace QLogger;
using namespace GitServer;

IRestApi::IRestApi(const ServerAuthentication &auth, QObject *parent)
   : QObject(parent)
   , mManager(new QNetworkAccessManager())
   , mAuth(auth)
{
}

IRestApi::~IRestApi()
{
   delete mManager;
}

QJsonDocument IRestApi::validateData(QNetworkReply *reply, QString &errorString)
{
   const auto data = reply->readAll();
   const auto jsonDoc = QJsonDocument::fromJson(data);

   if (reply->error() != QNetworkReply::NoError)
   {
      QLog_Error("Ui", QString("Error #%1 - %2.").arg(QString::number(reply->error()), reply->errorString()));

      errorString = reply->errorString();

      return QJsonDocument();
   }

   if (jsonDoc.isNull())
   {
      QLog_Error("Ui", QString("Error when parsing Json. Current data:\n%1").arg(QString::fromUtf8(data)));
      return QJsonDocument();
   }

   const auto jsonObject = jsonDoc.object();
   if (jsonObject.contains(QStringLiteral("message")))
   {
      const auto message = jsonObject[QStringLiteral("message")].toString();
      QString details;

      if (jsonObject.contains(QStringLiteral("errors")))
      {
         const auto errors = jsonObject[QStringLiteral("errors")].toArray();

         for (const auto &error : errors)
            details = error[QStringLiteral("message")].toString();

         errorString = message + ". " + details;

         QLog_Error("Ui", errorString);

         return QJsonDocument();
      }
   }
   else if (jsonObject.contains(QStringLiteral("error")))
   {
      errorString = jsonObject[QStringLiteral("error")].toString();

      QLog_Error("Ui", errorString);

      return QJsonDocument();
   }

   reply->deleteLater();

   return jsonDoc;
}
