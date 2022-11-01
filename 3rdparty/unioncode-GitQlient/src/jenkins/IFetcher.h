#pragma once

#include <QObject>

#include <QSharedPointer>

class QNetworkAccessManager;
class QNetworkReply;
class QJsonDocument;

namespace Jenkins
{
class IFetcher : public QObject
{
   Q_OBJECT
public:
   struct Config
   {
      QString user;
      QString token;
      QSharedPointer<QNetworkAccessManager> accessManager;
   };

   explicit IFetcher(const IFetcher::Config &config, QObject *parent = nullptr);
   ~IFetcher();

   virtual void triggerFetch() = 0;

protected:
   IFetcher::Config mConfig;

   virtual void get(const QString &urlStr, bool customUrl = false) final;

private:
   virtual void processReply() final;
   virtual void processData(const QJsonDocument &json) = 0;
};
}
