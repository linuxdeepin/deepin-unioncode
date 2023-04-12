/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef JSDebugger_H
#define JSDebugger_H

#include "base/abstractdebugger.h"

#include <QObject>
#include <QtScript/QScriptEngine>
#include <QScriptEngineDebugger>

class JSDebugger : public AbstractDebugger
{
    Q_OBJECT
public:
    explicit JSDebugger(QObject *parent = nullptr);
    ~JSDebugger() override{}

    QWidget *getOutputPane() const override;
    QWidget *getStackPane() const override;
    QWidget *getLocalsPane() const override;
    QWidget *getBreakpointPane() const override;

    void startDebug() override;
    void detachDebug() override;

    void interruptDebug() override;
    void continueDebug() override;
    void abortDebug() override;
    void restartDebug() override;

    void stepOver() override;
    void stepIn() override;
    void stepOut() override;

    RunState getRunState() const override;
    bool runCoredump(const QString &target, const QString &core, const QString &kit) override;

signals:

public slots:
    void slotEvaluationSuspended();
    void slotEvaluationResumed();
    void setupDebugEnv();

private:
    void runCommand(QScriptEngineDebugger::DebuggerAction command);
    QWidget *getWidget(QScriptEngineDebugger::DebuggerWidget widget) const;
    QScriptValue evaluateFile(const QString &filePath);

    QScriptEngine engine;
    QScriptEngineDebugger debugger;

    RunState runState = kNoRun;

    QWidget *oldWidgetEdit = nullptr;
    QWidget *oldWidgetWatch = nullptr;
};

#endif // JSDebugger_H
