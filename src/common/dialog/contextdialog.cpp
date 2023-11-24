// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contextdialog.h"

#include <DMessageBox>
#include <DDialog>
#include <DPushButton>

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QFile>
#include <QCheckBox>
#include <QGroupBox>
#include <QDebug>

void ContextDialog::okCancel(QString text, QString title,
                             DMessageBox::Icon icon,
                             std::function<void (bool)> okCallBack,
                             std::function<void (bool)> cancelCallBack)
{
    if (text.isEmpty())
        return;

    DMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(DMessageBox::Ok | DMessageBox::Cancel);
    messageBox.button(DMessageBox::Ok)->setText(QObject::tr("Ok"));
    messageBox.button(DMessageBox::Cancel)->setText(QObject::tr("Cancel"));
    if (okCallBack)
        QObject::connect(messageBox.button(DMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    if (cancelCallBack)
        QObject::connect(messageBox.button(DMessageBox::Cancel),
                         &QAbstractButton::clicked, cancelCallBack);
    messageBox.exec();
}

void ContextDialog::ok(QString text, QString title,
                       DMessageBox::Icon icon,
                       std::function<void (bool)> okCallBack)
{
    if (text.isEmpty())
        return;

    DMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(DMessageBox::Ok);
    messageBox.button(DMessageBox::Ok)->setText(QObject::tr("Ok"));
    if (okCallBack)
        QObject::connect(messageBox.button(DMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    messageBox.exec();
}

void ContextDialog::question(QString text, QString title, DMessageBox::Icon icon, std::function<void (bool)> okCallBack, std::function<void (bool)> noCallBack, std::function<void (bool)> cancelCallBack)
{
    if (text.isEmpty())
        return;

    DMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(DMessageBox::Ok | DMessageBox::No | DMessageBox::Cancel);
    messageBox.button(DMessageBox::Ok)->setText(QObject::tr("Ok"));
    messageBox.button(DMessageBox::No)->setText(QObject::tr("No"));
    messageBox.button(DMessageBox::Cancel)->setText(QObject::tr("Cancel"));
    if (okCallBack)
        QObject::connect(messageBox.button(DMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    if (noCallBack)
        QObject::connect(messageBox.button(DMessageBox::No),
                         &QAbstractButton::clicked, noCallBack);
    if (cancelCallBack)
        QObject::connect(messageBox.button(DMessageBox::Cancel),
                         &QAbstractButton::clicked, cancelCallBack);
    messageBox.exec();
}

void ContextDialog::singleChoice(QSet<SingleChoiceBox::Info> infos,
                                 QString windowTitle, QString choiceTitle,
                                 std::function<void (const SingleChoiceBox::Info&)> okCallBack,
                                 std::function<void (const SingleChoiceBox::Info&)> cancelCallBack)
{
    DDialog dialog;
    dialog.setWindowTitle(windowTitle);

    QHBoxLayout *hLayoutButton = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout;
    DPushButton *okPbt = new DPushButton(DPushButton::tr("Ok"));
    DPushButton *cancelPbt = new DPushButton(DPushButton::tr("Cancel"));

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

    QObject::connect(okPbt, &DPushButton::clicked, [=, &cacheInfo, &dialog](){
        if (okCallBack) {
            okCallBack(cacheInfo);
        }
        dialog.close();
    });

    QObject::connect(cancelPbt, &DPushButton::clicked, [=, &cacheInfo, &dialog](){
        if (cancelCallBack) {
            cancelCallBack(cacheInfo);
        }
        dialog.close();
    });

    dialog.exec();
}
