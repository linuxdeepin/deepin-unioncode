/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef CODEPORTING_H
#define CODEPORTING_H

#include "common/widget/outputpane.h"

#include <QProcess>
#include <QObject>

class CodePorting : public QObject
{
    Q_OBJECT
public:
    enum PortingStatus {
        kNoRuning,
        kRuning,
        kSuccessful,
        kFailed
    };

    enum ReportItems {
        kFilePath,
        kCodeRange,
        kKey,
        kSuggestion,
        kFileType,
        kItemsCount
    };

    using Report = QMap<QString, QList<QStringList>>;
    using ReportIterator = QMapIterator<QString, QList<QStringList>>;

    explicit CodePorting(QObject *parent = nullptr);

    bool start(const QString &projectSrcPath, const QString &srcCPU, const QString &buildDir, const QString &destCPU);
    bool abort();

    PortingStatus getStatus() const;
    bool isRunning();
    const Report &getReport() const;
    const QStringList &getSrcItemNames() const;
    const QStringList &getLibItemNames() const;

    const QList<QStringList> getSourceReport() const;
    const QList<QStringList> getDependLibReport() const;

signals:
    QString outputInformation(const QString &line, OutputPane::OutputFormat format, OutputPane::AppendMode mode = OutputPane::Normal);
    void notifyPortingStatus(PortingStatus status);

public slots:
private:
    QString getPython();
    void resetUI();
    void updateStatus(PortingStatus _status);
    QString parseReportPath(const QString &line);
    OutputPane::AppendMode parseFormat(const QString &line);
    bool parseReportFromFile(const QString &reportPath);

    QProcess process;

    QString pythonCmd;
    Report report;
    QString projSrcPath;

    PortingStatus status { kNoRuning };
};

#endif   // CODEPORTING_H
