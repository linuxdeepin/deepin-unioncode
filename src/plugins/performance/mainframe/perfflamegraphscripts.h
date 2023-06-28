// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERFFLAMEGRAPHSCRIPT_H
#define PERFFLAMEGRAPHSCRIPT_H

#include "common/common.h"

#include <QString>

#include <functional>

typedef QProcess Task;

class PerfRecord : public Task
{
    Q_OBJECT
public:
    PerfRecord(const QString &outFile);
    void setAttachPid(uint pid);
private:
    uint pid;
    QString ouFile;
};

class PerfScript : public Task
{
    Q_OBJECT
public:
    PerfScript(const QString &perfRecordOutFileIn, const QString &outFile);
};

class StackCollapse : public Task
{
    Q_OBJECT
public:
    StackCollapse(const QString &perfScriptOutFile, const QString &outFile);
};

// out file is svg fromat
class FlameGraph : public Task
{
    Q_OBJECT
public:
    FlameGraph(const QString &stackCollapseOutFile, const QString &outFile);
};

class FlameGraphGenTaskPrivate;
class FlameGraphGenTask : public QObject
{
    Q_OBJECT
    FlameGraphGenTaskPrivate *const d;
public:
    explicit FlameGraphGenTask(QObject *parent = nullptr);
    virtual ~FlameGraphGenTask();
    void showWebBrowser(bool flag);
    void start(uint pid);
    void stop();

signals:
    void started();
    void error(const QString &err);
    void showed(const QString svgFile);
};

#endif // PERFFLAMEGRAPHSCRIPT_H
