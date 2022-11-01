#include "AddRemoteDlg.h"
#include "ui_AddSubmoduleDlg.h"

#include <GitRemote.h>
#include <GitQlientStyles.h>

#include <QMessageBox>

AddRemoteDlg::AddRemoteDlg(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QDialog(parent)
   , ui(new Ui::AddSubmoduleDlg)
   , mGit(git)
{
   setStyleSheet(GitQlientStyles::getStyles());

   ui->setupUi(this);

   setWindowTitle("Add remote repository");

   connect(ui->lePath, &QLineEdit::returnPressed, this, &AddRemoteDlg::accept);
   connect(ui->leUrl, &QLineEdit::returnPressed, this, &AddRemoteDlg::accept);
   connect(ui->leUrl, &QLineEdit::editingFinished, this, &AddRemoteDlg::proposeName);
   connect(ui->pbAccept, &QPushButton::clicked, this, &AddRemoteDlg::accept);
   connect(ui->pbCancel, &QPushButton::clicked, this, &QDialog::reject);
}

AddRemoteDlg::~AddRemoteDlg()
{
   delete ui;
}

void AddRemoteDlg::accept()
{
   const auto remoteName = ui->lePath->text();
   const auto remoteUrl = ui->leUrl->text();

   QScopedPointer<GitRemote> git(new GitRemote(mGit));

   if (remoteName.isEmpty() || remoteUrl.isEmpty())
   {
      QMessageBox::warning(
          this, tr("Invalid fields"),
          tr("The information provided is incorrect. Please fix the URL and/or the name and submit again."));
   }
   else if (const auto ret = git->addRemote(remoteUrl, remoteName); ret.success)
   {
      git->fetch();

      QDialog::accept();
   }
}

void AddRemoteDlg::proposeName()
{
   auto url = ui->leUrl->text();
   QString proposedName;

   if (url.startsWith("https"))
   {
      url.remove("https://");
      const auto fields = url.split("/");

      if (fields.count() > 1)
         proposedName = fields.at(1);
   }
   else if (url.contains("@"))
   {
      const auto fields = url.split(":");

      if (fields.count() > 0)
         proposedName = fields.constFirst();
   }

   ui->lePath->setText(proposedName);
}
