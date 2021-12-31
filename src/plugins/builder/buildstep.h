/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef BUILDSTEP_H
#define BUILDSTEP_H

#include "builderglobals.h"

#include <QObject>
#include <QProcess>

class BuildStep : public QObject
{
    Q_OBJECT
public:
    explicit BuildStep(QObject *parent = nullptr);

    void setToolChainType(ToolChainType type);
    void setBuildOutputDir(const QString &outputDir);
    void setMakeFile(const QString &makeFile);
    void appendCmdParam(const QString param);

    virtual void run() = 0;

signals:
    void addOutput(const QString &content);

public slots:
    void processReadyReadStdOutput();
    void processReadyReadStdError();

protected:
    virtual void stdOutput(const QString &line);
    virtual void stdErrput(const QString &line);
    virtual bool execCmd(const QString &cmd, const QStringList &args);

    QString buildOutputDir;
    QString makeFile;
    QStringList cmdParams;
    ToolChainType toolChainType = {UnKnown};

    QScopedPointer<QProcess> process;
};

#endif // BUILDSTEP_H
