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
#include "contextdialog.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFile>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>

void ContextDialog::okCancel(QString text, QString title,
                             QMessageBox::Icon icon,
                             std::function<void (bool)> okCallBack,
                             std::function<void (bool)> cancelCallBack)
{
    QMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
    if (okCallBack)
        QObject::connect(messageBox.button(QMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    if (cancelCallBack)
        QObject::connect(messageBox.button(QMessageBox::Cancel),
                         &QAbstractButton::clicked, cancelCallBack);
    messageBox.exec();
}

void ContextDialog::ok(QString text, QString title,
                       QMessageBox::Icon icon,
                       std::function<void (bool)> okCallBack)
{
    QMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Ok);
    if (okCallBack)
        QObject::connect(messageBox.button(QMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    messageBox.exec();
}

#include <QDebug>
void ContextDialog::singleChoice(QSet<SingleChoiceBox::Info> infos,
                                 QString windowTitle, QString choiceTitle,
                                 std::function<void (const SingleChoiceBox::Info&)> okCallBack,
                                 std::function<void (const SingleChoiceBox::Info&)> cancelCallBack)
{
    QDialog dialog;
    dialog.setWindowTitle(windowTitle);

    QHBoxLayout *hLayoutButton = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout;
    QPushButton *okPbt = new QPushButton(QPushButton::tr("Ok"));
    QPushButton *cancelPbt = new QPushButton(QPushButton::tr("Cancel"));

    SingleChoiceBox::Info cacheInfo;
    SingleChoiceBox *box = new SingleChoiceBox;
    box->setChoiceTitle(choiceTitle);
    box->setInfos(infos);
    QObject::connect(box, &SingleChoiceBox::selected,
                     [&cacheInfo](const SingleChoiceBox::Info &info){
        cacheInfo = info;
    });

    vLayout->addWidget(box);
    vLayout->addLayout(hLayoutButton);
    dialog.setLayout(vLayout);

    hLayoutButton->addWidget(okPbt);
    hLayoutButton->addWidget(cancelPbt);

    QObject::connect(okPbt, &QPushButton::clicked, [=, &cacheInfo, &dialog](){
        if (okCallBack) {
            okCallBack(cacheInfo);
        }
        dialog.close();
    });

    QObject::connect(cancelPbt, &QPushButton::clicked, [=, &cacheInfo, &dialog](){
        if (cancelCallBack) {
            cancelCallBack(cacheInfo);
        }
        dialog.close();
    });

    dialog.exec();
}
