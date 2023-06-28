// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLS_H
#define TOOLS_H

#include <QProcess>

#include <json/json.h>

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
