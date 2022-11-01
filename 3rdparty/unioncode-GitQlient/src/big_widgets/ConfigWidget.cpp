#include "ConfigWidget.h"
#include "ui_ConfigWidget.h"

#include <FileEditor.h>
#include <GitBase.h>
#include <GitQlientSettings.h>
#include <QLogger.h>

#include <CredentialsDlg.h>
#include <GitConfig.h>
#include <GitCredentials.h>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>

using namespace QLogger;

namespace
{
qint64 dirSize(QString dirPath)
{
   qint64 size = 0;
   QDir dir(dirPath);

   auto entryList = dir.entryList(QDir::Files | QDir::System | QDir::Hidden);

   for (const auto &filePath : qAsConst(entryList))
   {
      QFileInfo fi(dir, filePath);
      size += fi.size();
   }

   entryList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden);

   for (const auto &childDirPath : qAsConst(entryList))
      size += dirSize(dirPath + QDir::separator() + childDirPath);

   return size;
}
}

ConfigWidget::ConfigWidget(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QWidget(parent)
   , ui(new Ui::ConfigWidget)
   , mGit(git)
   , mFeedbackTimer(new QTimer())
   , mSave(new QPushButton())
{
   ui->setupUi(this);

   mFeedbackTimer->setInterval(3000);

   mSave->setIcon(QIcon(":/icons/save"));
   mSave->setToolTip(tr("Save"));
   connect(mSave, &QPushButton::clicked, this, &ConfigWidget::saveFile);
   ui->tabWidget->setCornerWidget(mSave);

   ui->mainLayout->setColumnStretch(0, 1);
   ui->mainLayout->setColumnStretch(1, 3);

   const auto localGitLayout = new QVBoxLayout(ui->localGit);
   localGitLayout->setContentsMargins(QMargins());

   mLocalGit = new FileEditor(false, this);
   mLocalGit->editFile(mGit->getGitDir().append("/config"));
   localGitLayout->addWidget(mLocalGit);

   const auto globalGitLayout = new QVBoxLayout(ui->globalGit);
   globalGitLayout->setContentsMargins(QMargins());

   mGlobalGit = new FileEditor(false, this);
   mGlobalGit->editFile(
       QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation), ".gitconfig"));
   globalGitLayout->addWidget(mGlobalGit);

   GitQlientSettings settings(mGit->getGitDir());

   ui->chDevMode->setChecked(settings.localValue("DevMode", false).toBool());
   enableWidgets();

   // GitQlient configuration
   ui->chDisableLogs->setChecked(settings.globalValue("logsDisabled", true).toBool());
   ui->cbLogLevel->setCurrentIndex(settings.globalValue("logsLevel", static_cast<int>(LogLevel::Warning)).toInt());
   ui->spCommitTitleLength->setValue(settings.globalValue("commitTitleMaxLength", 50).toInt());

   const auto originalStyles = settings.globalValue("colorSchema", "dark").toString();
   ui->cbStyle->setCurrentText(originalStyles);
   connect(ui->cbStyle, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this,
           [this, originalStyles]() {
              mShowResetMsg = ui->cbStyle->currentText() != originalStyles;
              saveConfig();
           });

   // Repository configuration
   mOriginalRepoOrder = settings.localValue("GraphSortingOrder", 0).toInt();
   ui->cbLogOrder->setCurrentIndex(mOriginalRepoOrder);
   ui->autoFetch->setValue(settings.localValue("AutoFetch", 5).toInt());
   ui->pruneOnFetch->setChecked(settings.localValue("PruneOnFetch", true).toBool());
   ui->clangFormat->setChecked(settings.localValue("ClangFormatOnCommit", false).toBool());
   ui->updateOnPull->setChecked(settings.localValue("UpdateOnPull", false).toBool());
   ui->sbMaxCommits->setValue(settings.localValue("MaxCommits", 0).toInt());

   ui->tabWidget->setCurrentIndex(0);
   connect(ui->pbClearCache, &ButtonLink::clicked, this, &ConfigWidget::clearCache);

   ui->cbPomodoroEnabled->setChecked(settings.localValue("Pomodoro/Enabled", true).toBool());

   ui->cbStash->setChecked(settings.localValue("StashesHeader", true).toBool());
   ui->cbSubmodule->setChecked(settings.localValue("SubmodulesHeader", true).toBool());
   ui->cbSubtree->setChecked(settings.localValue("SubtreeHeader", true).toBool());

   // Build System configuration
   const auto isConfigured = settings.localValue("BuildSystemEnabled", false).toBool();
   ui->chBoxBuildSystem->setChecked(isConfigured);
   connect(ui->chBoxBuildSystem, &QCheckBox::stateChanged, this, &ConfigWidget::toggleBsAccesInfo);

   ui->leBsUser->setVisible(isConfigured);
   ui->leBsUserLabel->setVisible(isConfigured);
   ui->leBsToken->setVisible(isConfigured);
   ui->leBsTokenLabel->setVisible(isConfigured);
   ui->leBsUrl->setVisible(isConfigured);
   ui->leBsUrlLabel->setVisible(isConfigured);

   if (isConfigured)
   {
      const auto url = settings.localValue("BuildSystemUrl", "").toString();
      const auto user = settings.localValue("BuildSystemUser", "").toString();
      const auto token = settings.localValue("BuildSystemToken", "").toString();

      ui->leBsUrl->setText(url);
      ui->leBsUser->setText(user);
      ui->leBsToken->setText(token);
   }

   QScopedPointer<GitConfig> gitConfig(new GitConfig(git));

   const auto url = gitConfig->getServerUrl();
   ui->credentialsFrames->setVisible(url.startsWith("https"));

   connect(ui->buttonGroup, SIGNAL(buttonClicked(QAbstractButton *)), this,
           SLOT(onCredentialsOptionChanged(QAbstractButton *)));
   connect(ui->pbAddCredentials, &QPushButton::clicked, this, &ConfigWidget::showCredentialsDlg);

   // Connects for automatic save
   connect(ui->chDevMode, &CheckBox::stateChanged, this, &ConfigWidget::enableWidgets);
   connect(ui->chDisableLogs, &CheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->cbLogLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
   connect(ui->leGitPath, &QLineEdit::editingFinished, this, &ConfigWidget::saveConfig);
   connect(ui->spCommitTitleLength, SIGNAL(valueChanged(int)), this, SLOT(saveConfig()));
   connect(ui->cbTranslations, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
   connect(ui->sbMaxCommits, SIGNAL(valueChanged(int)), this, SLOT(saveConfig()));
   connect(ui->cbLogOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
   connect(ui->autoFetch, SIGNAL(valueChanged(int)), this, SLOT(saveConfig()));
   connect(ui->pruneOnFetch, &QCheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->updateOnPull, &QCheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->clangFormat, &QCheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->cbPomodoroEnabled, &QCheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->cbStash, &QCheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->cbSubmodule, &QCheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->cbSubtree, &QCheckBox::stateChanged, this, &ConfigWidget::saveConfig);
   connect(ui->leBsUrl, &QLineEdit::editingFinished, this, &ConfigWidget::saveConfig);
   connect(ui->leBsUser, &QLineEdit::editingFinished, this, &ConfigWidget::saveConfig);
   connect(ui->leBsToken, &QLineEdit::editingFinished, this, &ConfigWidget::saveConfig);

   calculateCacheSize();
}

ConfigWidget::~ConfigWidget()
{
   delete ui;
}

void ConfigWidget::onPanelsVisibilityChanged()
{
   GitQlientSettings settings(mGit->getGitDir());

   ui->cbStash->setChecked(settings.localValue("StashesHeader", true).toBool());
   ui->cbSubmodule->setChecked(settings.localValue("SubmodulesHeader", true).toBool());
   ui->cbSubtree->setChecked(settings.localValue("SubtreeHeader", true).toBool());
}

void ConfigWidget::onCredentialsOptionChanged(QAbstractButton *button)
{
   ui->sbTimeout->setEnabled(button == ui->rbCache);
}

void ConfigWidget::clearCache()
{
   const auto path = QString("%1").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
   QProcess p;
   p.setWorkingDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
   p.start("rm", { "-rf", path });

   if (p.waitForFinished())
      calculateCacheSize();
}

void ConfigWidget::calculateCacheSize()
{
   auto size = 0;
   const auto dirPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
   QDir dir(dirPath);
   QDir::Filters dirFilters = QDir::Dirs | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::Files;
   const auto &list = dir.entryInfoList(dirFilters);

   for (const QFileInfo &file : list)
   {
      size += file.size();
      size += dirSize(dirPath + "/" + file.fileName());
   }

   ui->lCacheSize->setText(QString("%1 KB").arg(size / 1024.0));
}

void ConfigWidget::toggleBsAccesInfo()
{
   const auto visible = ui->chBoxBuildSystem->isChecked();
   ui->leBsUser->setVisible(visible);
   ui->leBsUserLabel->setVisible(visible);
   ui->leBsToken->setVisible(visible);
   ui->leBsTokenLabel->setVisible(visible);
   ui->leBsUrl->setVisible(visible);
   ui->leBsUrlLabel->setVisible(visible);
}

void ConfigWidget::saveConfig()
{
   mFeedbackTimer->stop();

   ui->lFeedback->setText(tr("Changes saved"));

   GitQlientSettings settings(mGit->getGitDir());

   settings.setGlobalValue("logsDisabled", ui->chDisableLogs->isChecked());
   settings.setGlobalValue("logsLevel", ui->cbLogLevel->currentIndex());
   settings.setGlobalValue("commitTitleMaxLength", ui->spCommitTitleLength->value());
   settings.setGlobalValue("colorSchema", ui->cbStyle->currentText());
   settings.setGlobalValue("gitLocation", ui->leGitPath->text());

   emit commitTitleMaxLenghtChanged();

   if (mShowResetMsg)
   {
      QMessageBox::information(this, tr("Reset needed!"),
                               tr("You need to restart GitQlient to see the changes in the styles applid."));
   }

   const auto logger = QLoggerManager::getInstance();
   logger->overwriteLogLevel(static_cast<LogLevel>(ui->cbLogLevel->currentIndex()));

   if (ui->chDisableLogs->isChecked())
      logger->pause();
   else
      logger->resume();

   if (mOriginalRepoOrder != ui->cbLogOrder->currentIndex())
   {
      settings.setLocalValue("GraphSortingOrder", ui->cbLogOrder->currentIndex());
      emit reloadView();
   }

   settings.setLocalValue("AutoFetch", ui->autoFetch->value());
   settings.setLocalValue("PruneOnFetch", ui->pruneOnFetch->isChecked());
   settings.setLocalValue("ClangFormatOnCommit", ui->clangFormat->isChecked());
   settings.setLocalValue("UpdateOnPull", ui->updateOnPull->isChecked());
   settings.setLocalValue("MaxCommits", ui->sbMaxCommits->value());

   settings.setLocalValue("StashesHeader", ui->cbStash->isChecked());
   settings.setLocalValue("SubmodulesHeader", ui->cbSubmodule->isChecked());
   settings.setLocalValue("SubtreeHeader", ui->cbSubtree->isChecked());

   emit panelsVisibilityChaned();

   /* POMODORO CONFIG */
   settings.setLocalValue("Pomodoro/Enabled", ui->cbPomodoroEnabled->isChecked());

   /* BUILD SYSTEM CONFIG */

   const auto showBs = ui->chBoxBuildSystem->isChecked();
   const auto bsUser = ui->leBsUser->text();
   const auto bsToken = ui->leBsToken->text();
   const auto bsUrl = ui->leBsUrl->text();

   if (showBs && !bsUser.isEmpty() && !bsToken.isEmpty() && !bsUrl.isEmpty())
   {
      settings.setLocalValue("BuildSystemEnabled", showBs);
      settings.setLocalValue("BuildSystemUrl", bsUrl);
      settings.setLocalValue("BuildSystemUser", bsUser);
      settings.setLocalValue("BuildSystemToken", bsToken);
      emit buildSystemConfigured(showBs);
   }
   else
   {
      settings.setLocalValue("BuildSystemEnabled", false);
      emit buildSystemConfigured(false);
   }

   mFeedbackTimer->singleShot(3000, ui->lFeedback, &QLabel::clear);
}

void ConfigWidget::enableWidgets()
{
   const auto enable = ui->chDevMode->isChecked();

   GitQlientSettings settings(mGit->getGitDir());
   settings.setLocalValue("DevMode", enable);

   ui->tabWidget->setEnabled(enable);
}

void ConfigWidget::saveFile()
{
   const auto id = ui->tabWidget->currentIndex();

   if (id == 0)
      mLocalGit->saveFile();
   else
      mGlobalGit->saveFile();
}

void ConfigWidget::showCredentialsDlg()
{
   // Store credentials if allowed and the user checked the box
   if (ui->credentialsFrames->isVisible() && ui->chbCredentials->isChecked())
   {
      if (ui->rbCache->isChecked())
         GitCredentials::configureCache(ui->sbTimeout->value(), mGit);
      else
      {
         CredentialsDlg dlg(mGit, this);
         dlg.exec();
      }
   }
}
