// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
