#pragma once

#include <QDialog>

namespace Ui
{
class PomodoroConfigDlg;
}

class GitBase;

class PomodoroConfigDlg : public QDialog
{
   Q_OBJECT

public:
   explicit PomodoroConfigDlg(const QSharedPointer<GitBase> &git, QWidget *parent = nullptr);
   ~PomodoroConfigDlg();

   void accept() override;

private:
   Ui::PomodoroConfigDlg *ui;
   QSharedPointer<GitBase> mGit;
};
