/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "checkoutdialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QUrl>
#include <QDebug>

CheckoutDialog::CheckoutDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog (parent, f)
    , labelRepos(new QLabel)
    , labelLocal(new QLabel)
    , labelUser(new QLabel)
    , labelPasswd(new QLabel)
    , editRepos(new QLineEdit)
    , editLocal(new QLineEdit)
    , editUser(new QLineEdit)
    , editPasswd(new QLineEdit)
    , pbtSelLocal(new QPushButton)
    , pbtOk(new QPushButton)
    , vLayoutPbt(new QVBoxLayout)
    , gLayout(new QGridLayout)
{
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
    pbtSelLocal->setFixedSize(20, 20);

    pbtOk->setText(QPushButton::tr("Ok"));
    vLayoutPbt->addWidget(pbtOk);
    vLayoutPbt->setMargin(4);

    gLayout->addWidget(labelRepos, 0, 0);
    gLayout->addWidget(editRepos, 0, 1);
    gLayout->addWidget(labelLocal, 1, 0);
    gLayout->addWidget(editLocal, 1, 1);
    gLayout->addWidget(pbtSelLocal, 1, 2, Qt::AlignLeft);
    gLayout->addWidget(labelUser, 2, 0);
    gLayout->addWidget(editUser, 2, 1);
    gLayout->addWidget(labelPasswd, 3, 0);
    gLayout->addWidget(editPasswd, 3, 1);

    gLayout->addLayout(vLayoutPbt, 4, 1, 1, 3, Qt::AlignRight);
    setLayout(gLayout);

    QObject::connect(pbtSelLocal, &QPushButton::clicked, [=](){
        QString directory = QFileDialog::getExistingDirectory(this);
        if (!directory.isEmpty()) {
            editLocal->setText(directory);
        }
    });

    QObject::connect(pbtOk, &QPushButton::clicked, [=]() {
        checkoutRepos(editRepos->text(), editLocal->text(), editUser->text(), editPasswd->text());
        this->close();
    });
}
