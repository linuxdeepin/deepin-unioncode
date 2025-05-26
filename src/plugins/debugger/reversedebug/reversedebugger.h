// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REVERSEDEBUGGER_H
#define REVERSEDEBUGGER_H

#include "services/language/languageservice.h"
#include "common/widget/appoutputpane.h"

#include <QObject>

class ReverseDebugger : public QObject
{
    Q_OBJECT
public:
    explicit ReverseDebugger(QObject *parent = nullptr);

    void init();

public slots:
    void synOutputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);

signals:
    void startReplay(const QString &target);
    void recordDone();
    void recordFailed(const QString &err);

private:
    bool checkRRInstalled();
    void record();
    void replay();

    void outputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);

    bool execCommand(const dpfservice::RunCommandInfo &args);
};

#endif   // REVERSEDEBUGGER_H
