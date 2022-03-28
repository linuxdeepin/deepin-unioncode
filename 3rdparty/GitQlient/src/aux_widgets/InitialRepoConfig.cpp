#include "InitialRepoConfig.h"
#include "ui_InitialRepoConfig.h"

#include <CredentialsDlg.h>
#include <GitConfig.h>
#include <GitCredentials.h>
#include <GitQlientSettings.h>
#include <GitQlientStyles.h>

InitialRepoConfig::InitialRepoConfig(const QSharedPointer<GitBase> &git,
                                     const QSharedPointer<GitQlientSettings> &settings, QWidget *parent)
   : QDialog(parent)
   , ui(new Ui::InitialRepoConfig)
   , mGit(git)
   , mSettings(settings)
{
   setAttribute(Qt::WA_DeleteOnClose);

   ui->setupUi(this);

   setStyleSheet(GitQlientStyles::getInstance()->getStyles());

   ui->autoFetch->setValue(mSettings->localValue("AutoFetch", 5).toInt());
   ui->pruneOnFetch->setChecked(settings->localValue("PruneOnFetch", true).toBool());
   ui->updateOnPull->setChecked(settings->localValue("UpdateOnPull", false).toBool());
   ui->sbMaxCommits->setValue(settings->localValue("MaxCommits", 0).toInt());

   QScopedPointer<GitConfig> gitConfig(new GitConfig(git));

   const auto url = gitConfig->getServerUrl();
   ui->credentialsFrames->setVisible(url.startsWith("https"));

   connect(ui->buttonGroup, SIGNAL(buttonClicked(QAbstractButton *)), this,
           SLOT(onCredentialsOptionChanged(QAbstractButton *)));
}

InitialRepoConfig::~InitialRepoConfig()
{
   mSettings->setLocalValue("AutoFetch", ui->autoFetch->value());
   mSettings->setLocalValue("PruneOnFetch", ui->pruneOnFetch->isChecked());
   mSettings->setLocalValue("UpdateOnPull", ui->updateOnPull->isChecked());
   mSettings->setLocalValue("MaxCommits", ui->sbMaxCommits->value());

   delete ui;
}

void InitialRepoConfig::accept()
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

   QDialog::accept();
}

void InitialRepoConfig::onCredentialsOptionChanged(QAbstractButton *button)
{
   ui->sbTimeout->setEnabled(button == ui->rbCache);
}
