#include "GitConfigDlg.h"
#include "ui_GitConfigDlg.h"

#include <GitBase.h>
#include <GitConfig.h>
#include <GitQlientStyles.h>

#include <QCloseEvent>
#include <QKeyEvent>

GitConfigDlg::GitConfigDlg(const QSharedPointer<GitBase> &gitBase, QWidget *parent)
   : QDialog(parent)
   , ui(new Ui::GitConfigDlg)
   , mGit(gitBase)
{
   ui->setupUi(this);

   setWindowFlags(Qt::FramelessWindowHint);
   setStyleSheet(GitQlientStyles::getStyles());

   QScopedPointer<GitConfig> git(new GitConfig(mGit));

   const auto globalConfig = git->getGlobalUserInfo();
   ui->leGlobalEmail->setText(globalConfig.mUserEmail);
   ui->leGlobalName->setText(globalConfig.mUserName);

   const auto localConfig = git->getLocalUserInfo();
   ui->leLocalEmail->setText(localConfig.mUserEmail);
   ui->leLocalName->setText(localConfig.mUserName);

   connect(ui->checkBox, &CheckBox::stateChanged, this, &GitConfigDlg::copyGlobalSettings);
   connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GitConfigDlg::accept);
}

GitConfigDlg::~GitConfigDlg()
{
   delete ui;
}

void GitConfigDlg::keyPressEvent(QKeyEvent *e)
{
   const auto key = e->key();

   if (key == Qt::Key_Escape)
      return;

   QDialog::keyPressEvent(e);
}

void GitConfigDlg::closeEvent(QCloseEvent *e)
{
   if (!mPrepareToClose)
      e->ignore();
   else
      QDialog::closeEvent(e);
}

void GitConfigDlg::close()
{
   mPrepareToClose = true;

   QDialog::close();
}

void GitConfigDlg::accept()
{
   QScopedPointer<GitConfig> git(new GitConfig(mGit));

   git->setGlobalUserInfo({ ui->leGlobalEmail->text(), ui->leGlobalName->text() });
   git->setLocalUserInfo({ ui->leLocalEmail->text(), ui->leLocalName->text() });

   close();
}

void GitConfigDlg::copyGlobalSettings(int checkState)
{
   ui->leLocalEmail->setReadOnly(checkState == Qt::Checked);
   ui->leLocalName->setReadOnly(checkState == Qt::Checked);

   if (checkState == Qt::Checked)
   {
      ui->leLocalEmail->setText(ui->leGlobalEmail->text());
      ui->leLocalName->setText(ui->leGlobalName->text());
   }
}
