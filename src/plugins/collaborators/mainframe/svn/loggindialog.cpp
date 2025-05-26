// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loggindialog.h"

LogginDialog::LogginDialog(QWidget *parent)
    : DDialog(parent)
    , titleLabel(new DLabel())
    , nameEdit(new DLineEdit())
    , passwdEdit(new DLineEdit())
    , pbtOk(new DPushButton(QPushButton::tr("Ok")))
{
    auto mainwidget = new DWidget;
    addContent(mainwidget);

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
    QObject::connect(nameEdit, &DLineEdit::textChanged, [=](const QString &text){
        Q_UNUSED(text);
        if (!nameEdit->lineEdit()->text().isEmpty() && !passwdEdit->lineEdit()->text().isEmpty()){
            pbtOk->setEnabled(true);
        } else {
            pbtOk->setEnabled(false);
        }
    });
    QObject::connect(passwdEdit, &DLineEdit::textChanged, [=](const QString &text){
        Q_UNUSED(text);
        if (!nameEdit->lineEdit()->text().isEmpty() && !passwdEdit->lineEdit()->text().isEmpty()){
            pbtOk->setEnabled(true);
        } else {
            pbtOk->setEnabled(false);
        }
    });
    QObject::connect(pbtOk, &DPushButton::pressed, this, &LogginDialog::logginOk);

    hLayoutPbt->addStrut(10);
    hLayoutPbt->addWidget(pbtOk);
    hLayoutPbt->setAlignment(Qt::AlignRight);
    vLayoutMain->addWidget(titleLabel);
    vLayoutMain->addWidget(nameEdit);
    vLayoutMain->addWidget(passwdEdit);
    vLayoutMain->addLayout(hLayoutPbt);
    mainwidget->setLayout(vLayoutMain);
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
