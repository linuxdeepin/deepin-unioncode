// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RUNNER_H
#define RUNNER_H

#include "event/event.h"
#include "services/language/languagegenerator.h"
#include "debuggerglobals.h"
#include "common/widget/appoutputpane.h"

#include <QObject>
#include <QAction>

class RunnerPrivate;
class Runner : public QObject
{
    Q_OBJECT
public:
    explicit Runner(QObject *parent = nullptr);
    ~Runner();

public slots:
    void run();
    void handleEvents(const dpf::Event &event);
    void synOutputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);

private:
    void running();
    bool execCommand(const dpfservice::RunCommandInfo &info);
    void outputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);
    dpfservice::ProjectInfo getActiveProjectInfo() const;

private:
   RunnerPrivate *const d;
};

#endif // RUNNER_H
