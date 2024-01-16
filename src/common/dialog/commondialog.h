// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONDIALOG_H
#define COMMONDIALOG_H

#include "common/widget/singlechoicebox.h"

#include <DMessageBox>

#include <QIcon>
#include <QString>
#include <QSet>
#include <QObject>

#include <functional>

DWIDGET_USE_NAMESPACE

class CommonDialog final
{
    Q_DISABLE_COPY(CommonDialog)
    CommonDialog() = delete;
public:
    static void okCancel(QString text,
                         QString title = "Warning",
                         DMessageBox::Icon icon = DMessageBox::Warning,
                         std::function<void(bool)> okCallBack = nullptr,
                         std::function<void(bool)> cancelCallBack = nullptr);

    static void ok(QString text,
                   QString title = "Error",
                   DMessageBox::Icon icon = DMessageBox::Critical,
                   std::function<void(bool)> okCallBack = nullptr);

    static void question(QString text,
                         QString title = "Question",
                         DMessageBox::Icon icon = DMessageBox::Question,
                         std::function<void(bool)> okCallBack = nullptr,
                         std::function<void(bool)> noCallBack = nullptr,
                         std::function<void(bool)> cancelCallBack = nullptr);

    static void singleChoice(QSet<SingleChoiceBox::Info> infos,
                             QString windowTitle = "Infos Selection",
                             QString choiceTitle = "Single Choice",
                             std::function<void(const SingleChoiceBox::Info&)> okCallBack = nullptr,
                             std::function<void(const SingleChoiceBox::Info&)> cancelCallBack = nullptr);
};

#endif
