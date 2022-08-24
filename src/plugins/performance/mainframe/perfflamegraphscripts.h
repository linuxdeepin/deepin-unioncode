/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
