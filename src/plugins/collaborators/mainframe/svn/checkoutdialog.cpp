// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkoutdialog.h"

#include <DFrame>

#include <QGridLayout>
#include <QFileDialog>
#include <QUrl>
#include <QDebug>

CheckoutDialog::CheckoutDialog(QWidget *parent, Qt::WindowFlags f)
    : DDialog (parent)
    , labelRepos(new DLabel)
    , labelLocal(new DLabel)
    , labelUser(new DLabel)
    , labelPasswd(new DLabel)
    , editRepos(new DLineEdit)
    , editLocal(new DLineEdit)
    , editUser(new DLineEdit)
    , editPasswd(new DLineEdit)
    , pbtSelLocal(new DPushButton)
    , pbtOk(new DPushButton)
    , vLayoutPbt(new QVBoxLayout)
    , gLayout(new QGridLayout)
{
    auto *mainframe = new DWidget(this);
    mainframe->setLayout(gLayout);
    addContent(mainframe);

    this->setWindowTitle(QDialog::tr("Checkout Repos"));
    this->setMinimumWidth(500);

    labelRepos->setText(QLabel::tr("Remote Repos: "));
    labelRepos->setAlignment(Qt::AlignRight);
    labelLocal->setText(QLabel::tr("Target path: "));
    labelLocal->setAlignment(Qt::AlignRight);
    labelUser->setText(QLabel::tr("User: "));
    labelUser->setAlignment(Qt::AlignRight);
    labelPasswd->setText(QLabel::tr("Password: "));
    labelPasswd->setAlignment(Qt::AlignRight);

    editPasswd->setEchoMode(QLineEdit::EchoMode::Password);

    pbtSelLocal->setText("...");

    pbtOk->setText(QPushButton::tr("Ok"));
    vLayoutPbt->addWidget(pbtOk);
    vLayoutPbt->setMargin(4);

    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(editLocal);
    hlayout->addWidget(pbtSelLocal);

    gLayout->addWidget(labelRepos, 0, 0);
    gLayout->addWidget(editRepos, 0, 1);
    gLayout->addWidget(labelLocal, 1, 0);
    gLayout->addLayout(hlayout, 1, 1);
    gLayout->addWidget(labelUser, 2, 0);
    gLayout->addWidget(editUser, 2, 1);
    gLayout->addWidget(labelPasswd, 3, 0);
    gLayout->addWidget(editPasswd, 3, 1);

    gLayout->addLayout(vLayoutPbt, 4, 1, 1, 3, Qt::AlignRight);

    QObject::connect(pbtSelLocal, &DPushButton::clicked, [=](){
        QString directory = QFileDialog::getExistingDirectory(this);
        if (!directory.isEmpty()) {
            editLocal->setText(directory);
        }
    });

    QObject::connect(pbtOk, &DPushButton::clicked, [=]() {
        checkoutRepos(editRepos->text(), editLocal->text(), editUser->text(), editPasswd->text());
        this->close();
    });
}
