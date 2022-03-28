#include "CredentialsDlg.h"
#include "ui_CredentialsDlg.h"

#include <GitCredentials.h>

CredentialsDlg::CredentialsDlg(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QDialog(parent)
   , ui(new Ui::CredentialsDlg)
   , mGit(git)
{
   ui->setupUi(this);
}

CredentialsDlg::~CredentialsDlg()
{
   delete ui;
}

void CredentialsDlg::accept()
{
   const auto username = ui->leUser->text();
   const auto password = ui->lePass->text();

   if (!username.isEmpty() && !password.isEmpty())
      GitCredentials::configureStorage(username, password, mGit);

   QDialog::accept();
}
