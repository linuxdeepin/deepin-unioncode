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
#ifndef TOOLS_H
#define TOOLS_H

#include <QProcess>

#include <jsoncpp/json/json.h>

class ToolsPrivate;
class Tools : public QObject
{
    Q_OBJECT
    ToolsPrivate *const d;
public:
    enum PerformanceType
    {
        function,
        global,
        vfs,
        socket,
        deviceIO,
        networkIO,
        checkMemory,
    };
    Q_ENUM(PerformanceType)

    Tools();
    virtual ~Tools();
    Q_INVOKABLE Json::Value data() const;

public slots:
    void startAll();
    void stopAll();
    void setAttachPID(int pid);
    int attachPID() const;

protected:
    virtual bool initTool(PerformanceType pt);
    virtual QRegularExpression lineRegExpRule(PerformanceType pt);

signals:
    void attachData(const Json::Value &);

private slots:
    void readAllStandardOutput();
    void readAllStandardError();
    void errorOccurred(QProcess::ProcessError error);
};


#endif // TOOLS_H
