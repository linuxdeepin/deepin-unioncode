// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    QFont font1;
    font1.setBold(true);
    font1.setWeight(QFont::Bold);
    titleLabel->setFont(font1);
    titleLabel->setObjectName("HeaderTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    pbtOk->setFixedWidth(60);
    pbtOk->setEnabled(false);
    nameEdit->setPlaceholderText("User");
    passwdEdit->setPlaceholderText("Password");
    passwdEdit->setEchoMode(QLineEdit::EchoMode::Password);
    QObject::connect(nameEdit, &QLineEdit::textChanged, [=](const QString &text){
        Q_UNUSED(text);
        if (!nameEdit->text().isEmpty() && !passwdEdit->text().isEmpty()){
            pbtOk->setEnabled(true);
        } else {
            pbtOk->setEnabled(false);
        }
    });
    QObject::connect(passwdEdit, &QLineEdit::textChanged, [=](const QString &text){
        Q_UNUSED(text);
        if (!nameEdit->text().isEmpty() && !passwdEdit->text().isEmpty()){
            pbtOk->setEnabled(true);
        } else {
            pbtOk->setEnabled(false);
        }
    });
    QObject::connect(pbtOk, &QPushButton::pressed, this, &LogginDialog::logginOk);

    hLayoutPbt->addStrut(10);
    hLayoutPbt->addWidget(pbtOk);
    hLayoutPbt->setAlignment(Qt::AlignRight);
    vLayoutMain->addWidget(titleLabel);
    vLayoutMain->addWidget(nameEdit);
    vLayoutMain->addWidget(passwdEdit);
    vLayoutMain->addLayout(hLayoutPbt);
    setLayout(vLayoutMain);
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
