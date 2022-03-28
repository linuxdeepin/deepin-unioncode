#include "PomodoroConfigDlg.h"
#include "ui_PomodoroConfigDlg.h"

#include <GitBase.h>
#include <GitQlientSettings.h>
#include <GitQlientStyles.h>

PomodoroConfigDlg::PomodoroConfigDlg(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QDialog(parent)
   , ui(new Ui::PomodoroConfigDlg)
   , mGit(git)
{
   ui->setupUi(this);

   connect(ui->pomodoroDur, &QSlider::valueChanged, this,
           [this](int value) { ui->pomodoroDurLabel->setText(QString::number(value)); });
   connect(ui->breakDur, &QSlider::valueChanged, this,
           [this](int value) { ui->pomodoroBreakDurLabel->setText(QString::number(value)); });
   connect(ui->longBreakDur, &QSlider::valueChanged, this,
           [this](int value) { ui->pomodoroLongBreakLabel->setText(QString::number(value)); });

   GitQlientSettings settings(mGit->getGitDir());
   ui->cbAlarmSound->setChecked(settings.localValue("Pomodoro/Alarm", false).toBool());
   ui->cbStopResets->setChecked(settings.localValue("Pomodoro/StopResets", true).toBool());
   ui->pomodoroDur->setValue(settings.localValue("Pomodoro/Duration", 25).toInt());
   ui->breakDur->setValue(settings.localValue("Pomodoro/Break", 5).toInt());
   ui->longBreakDur->setValue(settings.localValue("Pomodoro/LongBreak", 15).toInt());
   ui->sbLongBreakCount->setValue(settings.localValue("Pomodoro/LongBreakTrigger", 4).toInt());

   setStyleSheet(GitQlientStyles::getInstance()->getStyles());
}

PomodoroConfigDlg::~PomodoroConfigDlg()
{
   delete ui;
}

void PomodoroConfigDlg::accept()
{
   GitQlientSettings settings(mGit->getGitDir());
   settings.setLocalValue("Pomodoro/Alarm", ui->cbAlarmSound->isChecked());
   settings.setLocalValue("Pomodoro/StopResets", ui->cbStopResets->isChecked());
   settings.setLocalValue("Pomodoro/Duration", ui->pomodoroDur->value());
   settings.setLocalValue("Pomodoro/Break", ui->breakDur->value());
   settings.setLocalValue("Pomodoro/LongBreak", ui->longBreakDur->value());
   settings.setLocalValue("Pomodoro/LongBreakTrigger", ui->sbLongBreakCount->value());

   QDialog::accept();
}
