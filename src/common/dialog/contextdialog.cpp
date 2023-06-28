// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contextdialog.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFile>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QDebug>

void ContextDialog::okCancel(QString text, QString title,
                             QMessageBox::Icon icon,
                             std::function<void (bool)> okCallBack,
                             std::function<void (bool)> cancelCallBack)
{
    if (text.isEmpty())
        return;

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
    if (text.isEmpty())
        return;

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
