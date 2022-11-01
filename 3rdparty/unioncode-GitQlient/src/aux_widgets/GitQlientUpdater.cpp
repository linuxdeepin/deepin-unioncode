#include "GitQlientUpdater.h"

#include <GitQlientStyles.h>
#include <QLogger.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QTimer>

using namespace QLogger;

GitQlientUpdater::GitQlientUpdater(QObject *parent)
   : QObject(parent)
   , mManager(new QNetworkAccessManager())
{
}

GitQlientUpdater::~GitQlientUpdater()
{
   delete mManager;
}

void GitQlientUpdater::checkNewGitQlientVersion()
{
   QNetworkRequest request;
   request.setRawHeader("User-Agent", "GitQlient");
   request.setRawHeader("X-Custom-User-Agent", "GitQlient");
   request.setRawHeader("Content-Type", "application/json");
   request.setUrl(QUrl("https://github.com/francescmm/ci-utils/releases/download/gq_update/updates.json"));
   request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

   const auto reply = mManager->get(request);
   connect(reply, &QNetworkReply::finished, this, &GitQlientUpdater::processUpdateFile);
}

void GitQlientUpdater::showInfoMessage()
{
   QMessageBox msgBox(
       QMessageBox::Information, tr("New version of GitQlient!"),
       QString(tr("There is a new version of GitQlient available. Your current version is {%1} and the new "
                  "one is {%2}. You can read more about the new changes in the detailed description."))
           .arg(VER, mLatestGitQlient),
       QMessageBox::Ok | QMessageBox::Close, qobject_cast<QWidget *>(parent()));
   msgBox.setButtonText(QMessageBox::Ok, tr("Download"));
   msgBox.setDetailedText(mChangeLog);
   msgBox.setStyleSheet(GitQlientStyles::getStyles());

   if (msgBox.exec() == QMessageBox::Ok)
      downloadFile();
}

void GitQlientUpdater::processUpdateFile()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   const auto data = reply->readAll();
   const auto jsonDoc = QJsonDocument::fromJson(data);

   if (jsonDoc.isNull())
   {
      QLog_Error("Ui", QString("Error when parsing Json. Current data:\n%1").arg(QString::fromUtf8(data)));
      return;
   }

   const auto json = jsonDoc.object();

   mLatestGitQlient = json["latest-version"].toString();
   const auto changeLogUrl = json["changelog"].toString();

   QJsonObject os;
   auto platformSupported = true;
#if defined(Q_OS_WIN)
   os = json["windows"].toObject();
#elif defined(Q_OS_LINUX)
   os = json["linux"].toObject();
#elif defined(Q_OS_OSX)
   os = json["osx"].toObject();
#else
   platformSupported = false;
   QLog_Error("Ui", QString("Platform not supported for updates"));
#endif

   const auto curVersion = QString("%1").arg(VER).split(".");

   if (curVersion.count() == 1)
      return;

   const auto newVersion = mLatestGitQlient.split(".");
   const auto nv = newVersion.at(0).toInt() * 10000 + newVersion.at(1).toInt() * 100 + newVersion.at(2).toInt();
   const auto cv = curVersion.at(0).toInt() * 10000 + curVersion.at(1).toInt() * 100 + curVersion.at(2).toInt();

   if (nv > cv)
   {
      if (!platformSupported)
      {
         QMessageBox::information(
             qobject_cast<QWidget *>(parent()), tr("New version available!"),
             tr("There is a new version of GitQlient available but your OS doesn't have a binary built. If you want to "
                "get the latest version, please <a href='https://github.com/francescmm/GitQlient/releases/tag/v%1'>get "
                "the source code from GitHub</a>.")
                 .arg(mLatestGitQlient));
      }
      else
      {
         mGitQlientDownloadUrl = os["download-url"].toString();
         emit newVersionAvailable();

         QTimer::singleShot(200, this, [this, changeLogUrl] {
            QNetworkRequest request;
            request.setRawHeader("User-Agent", "GitQlient");
            request.setRawHeader("X-Custom-User-Agent", "GitQlient");
            request.setRawHeader("Content-Type", "application/json");
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
            request.setUrl(QUrl(changeLogUrl));

            const auto reply = mManager->get(request);

            connect(reply, &QNetworkReply::finished, this, &GitQlientUpdater::processChangeLog);
         });
      }
   }
}

void GitQlientUpdater::processChangeLog()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   mChangeLog = QString::fromUtf8(reply->readAll());
}

void GitQlientUpdater::downloadFile()
{
   QNetworkRequest request;
   request.setRawHeader("User-Agent", "GitQlient");
   request.setRawHeader("X-Custom-User-Agent", "GitQlient");
   request.setRawHeader("Content-Type", "application/octet-stream");
   request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
   request.setUrl(QUrl(mGitQlientDownloadUrl));

   const auto fileName = mGitQlientDownloadUrl.split("/").last();

   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 read, qint64 total) {
      if (mDownloadLog == nullptr)
      {
         mDownloadLog
             = new QProgressDialog(tr("Downloading..."), tr("Close"), 0, total, qobject_cast<QWidget *>(parent()));
         mDownloadLog->setAttribute(Qt::WA_DeleteOnClose);
         mDownloadLog->setAutoClose(false);
         mDownloadLog->setAutoReset(false);
         mDownloadLog->setMaximum(total);
         mDownloadLog->setCancelButton(nullptr);
         mDownloadLog->setWindowFlag(Qt::FramelessWindowHint);

         connect(mDownloadLog, &QProgressDialog::destroyed, this, [this]() { mDownloadLog = nullptr; });
      }

      mDownloadLog->setValue(read);
      mDownloadLog->show();
   });

   connect(reply, &QNetworkReply::finished, this, [this, reply, fileName]() {
      mDownloadLog->close();
      mDownloadLog = nullptr;

      const auto b = reply->readAll();
      const auto destination = QString("%1/%2").arg(
          QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).constFirst(), fileName);
      QFile file(destination);
      if (file.open(QIODevice::WriteOnly))
      {
         QDataStream out(&file);
         out << b;

         file.close();
      }

      reply->deleteLater();
   });
}
