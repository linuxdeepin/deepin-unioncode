// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTEXTDIALOG_H
#define CONTEXTDIALOG_H

#include "common/widget/singlechoicebox.h"

#include <QIcon>
#include <QString>
#include <QMessageBox>
#include <QSet>

#include <functional>

class ContextDialog final
{
    Q_DISABLE_COPY(ContextDialog)
    ContextDialog() = delete;
public:
    static void okCancel(QString text,
                         QString title = "Warning",
                         QMessageBox::Icon icon = QMessageBox::Warning,
                         std::function<void(bool)> okCallBack = nullptr,
                         std::function<void(bool)> cancelCallBack = nullptr);

    static void ok(QString text,
                   QString title = "Error",
                   QMessageBox::Icon icon = QMessageBox::Critical,
                   std::function<void(bool)> okCallBack = nullptr);

    static void singleChoice(QSet<SingleChoiceBox::Info> infos,
                             QString windowTitle = "Infos Selection",
                             QString choiceTitle = "Single Choice",
                             std::function<void(const SingleChoiceBox::Info&)> okCallBack = nullptr,
                             std::function<void(const SingleChoiceBox::Info&)> cancelCallBack = nullptr);
};

#endif
