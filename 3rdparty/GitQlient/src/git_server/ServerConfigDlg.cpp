#include "ServerConfigDlg.h"
#include "ui_ServerConfigDlg.h"

#include <GitBase.h>
#include <GitConfig.h>
#include <GitQlientStyles.h>
#include <GitQlientSettings.h>
#include <GitHubRestApi.h>
#include <GitLabRestApi.h>
#include <GitServerCache.h>

#include <QLogger.h>

#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>
#include <QDesktopServices>

#include <utility>

using namespace QLogger;
using namespace GitServer;

namespace
{
enum GitServerPlatform
{
   GitHub,
   GitHubEnterprise,
   GitLab,
   Bitbucket
};

static const QMap<GitServerPlatform, const char *> repoUrls { { GitHub, "https://api.github.com" },
                                                              { GitHubEnterprise, "" },
                                                              { GitLab, "https://gitlab.com/api/v4" } };
}

ServerConfigDlg::ServerConfigDlg(const QSharedPointer<GitServerCache> &gitServerCache,
                                 const GitServer::ConfigData &data, QWidget *parent)
   : QDialog(parent)
   , ui(new Ui::ServerConfigDlg)
   , mGitServerCache(gitServerCache)
   , mData(data)
   , mManager(new QNetworkAccessManager())
{
   setStyleSheet(GitQlientStyles::getStyles());

   ui->setupUi(this);

   connect(ui->cbServer, &QComboBox::currentTextChanged, this, &ServerConfigDlg::onServerChanged);

   ui->leEndPoint->setHidden(true);

   GitQlientSettings settings("");
   ui->leUserName->setText(mData.user);
   ui->leUserToken->setText(mData.token);
   ui->leEndPoint->setText(
       settings.globalValue(QString("%1/endpoint").arg(mData.serverUrl), repoUrls.value(GitHub)).toString());

   ui->cbServer->insertItem(GitHub, "GitHub", repoUrls.value(GitHub));
   ui->cbServer->insertItem(GitHubEnterprise, "GitHub Enterprise", repoUrls.value(GitHubEnterprise));

   if (mData.serverUrl.contains("github"))
   {
      const auto index = repoUrls.key(ui->leEndPoint->text().toUtf8(), GitHubEnterprise);
      ui->cbServer->setCurrentIndex(index);
   }
   else
   {
      ui->cbServer->insertItem(GitLab, "GitLab", repoUrls.value(GitLab));
      ui->cbServer->setCurrentIndex(GitLab);
      ui->cbServer->setVisible(false);
   }

   ui->lAccessToken->setText(tr("How to get a token?"));
   connect(ui->lAccessToken, &ButtonLink::clicked, [this]() {
      const auto url = mData.serverUrl.contains("github")
          ? "https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token"
          : "https://docs.gitlab.com/ee/user/profile/personal_access_tokens.html";
      QDesktopServices::openUrl(QUrl(QString::fromUtf8(url)));
   });

   connect(ui->leUserToken, &QLineEdit::editingFinished, this, &ServerConfigDlg::checkToken);
   connect(ui->leUserToken, &QLineEdit::returnPressed, this, &ServerConfigDlg::accept);
   connect(ui->pbAccept, &QPushButton::clicked, this, &ServerConfigDlg::accept);
   connect(ui->pbCancel, &QPushButton::clicked, this, &ServerConfigDlg::reject);
   connect(ui->pbTest, &QPushButton::clicked, this, &ServerConfigDlg::testToken);
}

ServerConfigDlg::~ServerConfigDlg()
{
   delete mManager;
   delete ui;
}

void ServerConfigDlg::checkToken()
{
   if (ui->leUserToken->text().isEmpty())
      ui->leUserName->setStyleSheet("border: 1px solid red;");
}

void ServerConfigDlg::accept()
{
   const auto endpoint = ui->cbServer->currentIndex() == GitHubEnterprise ? ui->leEndPoint->text()
                                                                          : ui->cbServer->currentData().toString();
   GitQlientSettings settings("");
   settings.setGlobalValue(QString("%1/user").arg(mData.serverUrl), ui->leUserName->text());
   settings.setGlobalValue(QString("%1/token").arg(mData.serverUrl), ui->leUserToken->text());
   settings.setGlobalValue(QString("%1/endpoint").arg(mData.serverUrl), endpoint);

   connect(mGitServerCache.get(), &GitServerCache::errorOccurred, this, &ServerConfigDlg::onGitServerError);
   connect(mGitServerCache.get(), &GitServerCache::connectionTested, this, [this]() { onDataValidated(); });

   mGitServerCache->init(mData.serverUrl, mData.repoInfo);
}

void ServerConfigDlg::testToken()
{
   if (ui->leUserToken->text().isEmpty())
      ui->leUserName->setStyleSheet("border: 1px solid red;");
   else
   {
      const auto endpoint = ui->cbServer->currentIndex() == GitHubEnterprise ? ui->leEndPoint->text()
                                                                             : ui->cbServer->currentData().toString();
      IRestApi *api = nullptr;

      if (ui->cbServer->currentIndex() == GitLab)
      {
         api = new GitLabRestApi(ui->leUserName->text(), mData.repoInfo.second, mData.serverUrl,
                                 { ui->leUserName->text(), ui->leUserToken->text(), endpoint }, this);
      }
      else
      {
         api = new GitHubRestApi(mData.repoInfo.first, mData.repoInfo.second,
                                 { ui->leUserName->text(), ui->leUserToken->text(), endpoint }, this);
      }

      api->testConnection();

      connect(api, &IRestApi::connectionTested, this, &ServerConfigDlg::onTestSucceeded);
      connect(api, &IRestApi::errorOccurred, this, &ServerConfigDlg::onGitServerError);
   }
}

void ServerConfigDlg::onServerChanged()
{
   ui->leEndPoint->setVisible(ui->cbServer->currentIndex() == GitHubEnterprise);
}

void ServerConfigDlg::onTestSucceeded()
{
   ui->lTestResult->setText(tr("Token confirmed!"));
   QTimer::singleShot(3000, ui->lTestResult, &QLabel::clear);
}

void ServerConfigDlg::onGitServerError(const QString &error)
{
   QMessageBox::warning(this, tr("API access error!"), error);
}

void ServerConfigDlg::onDataValidated()
{
   emit configured();

   QDialog::accept();
}
