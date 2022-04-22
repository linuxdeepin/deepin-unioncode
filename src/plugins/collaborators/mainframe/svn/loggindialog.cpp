#include "loggindialog.h"

#include <QPushButton>

LogginDialog::LogginDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , titleLabel(new QLabel())
    , nameEdit(new QLineEdit())
    , passwdEdit(new QLineEdit())
    , pbtOk(new QPushButton(QPushButton::tr("Ok")))
{
    setWindowFlag(Qt::FramelessWindowHint);
    QHBoxLayout *hLayoutPbt = new QHBoxLayout();
    QVBoxLayout *vLayoutMain = new QVBoxLayout();

    setFixedWidth(300);
    titleLabel->setAlignment(Qt::AlignCenter);
    pbtOk->setFixedWidth(60);
    nameEdit->setPlaceholderText("User Name");
    passwdEdit->setPlaceholderText("Password");
    passwdEdit->setEchoMode(QLineEdit::EchoMode::Password);
    hLayoutPbt->addStrut(10);
    hLayoutPbt->addWidget(pbtOk);
    hLayoutPbt->setAlignment(Qt::AlignRight);
    vLayoutMain->addWidget(titleLabel);
    vLayoutMain->addWidget(nameEdit);
    vLayoutMain->addWidget(passwdEdit);
    vLayoutMain->addLayout(hLayoutPbt);
    setLayout(vLayoutMain);

    QObject::connect(pbtOk, &QPushButton::pressed, [=](){
        this->close();
    });
}

void LogginDialog::setName(const QString &name)
{
    nameEdit->setText(name);
}

QString LogginDialog::name() const
{
    return nameEdit->text();
}

void LogginDialog::setPasswd(const QString &name)
{
    passwdEdit->setText(name);
}

QString LogginDialog::passwd() const
{
    return passwdEdit->text();
}

void LogginDialog::setTitleText(const QString &name)
{
    titleLabel->setText(name);
}

QString LogginDialog::titleText() const
{
    return titleLabel->text();
}
