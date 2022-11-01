#pragma once

#include <QObject>

class QNetworkAccessManager;
class QProgressDialog;

class GitQlientUpdater : public QObject
{
   Q_OBJECT
signals:
   void newVersionAvailable();

public:
   explicit GitQlientUpdater(QObject *parent = nullptr);
   ~GitQlientUpdater();

   void checkNewGitQlientVersion();
   void showInfoMessage();

private:
   QNetworkAccessManager *mManager = nullptr;
   QProgressDialog *mDownloadLog = nullptr;
   QString mLatestGitQlient;
   QString mChangeLog;
   QString mGitQlientDownloadUrl;

   void processUpdateFile();
   void processChangeLog();
   void downloadFile();
};
