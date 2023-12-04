// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEPORTINGMANAGER_H
#define CODEPORTINGMANAGER_H

#include "codeporting.h"
#include "codeportingwidget.h"

#include <QObject>

class ConfigWidget;
class OutputPane;
class ReportPane;
class CodePortingManager : public QObject
{
    Q_OBJECT
public:
    static CodePortingManager *instance();
    OutputPane *getOutputPane() const;
    ReportPane *getReportPane() const;

signals:

public slots:
    void slotShowConfigWidget();
    void slotPortingStart(const QString &project, const QString &srcCPU, const QString &destCPU);
    void slotAppendOutput(const QString &, OutputPane::OutputFormat format, OutputPane::AppendMode);
    void slotPortingStatus(CodePorting::PortingStatus status);
    void slotSelectedChanged(const QString &filePath, const QString &suggestion, int startLine, int endLine);

private:
    explicit CodePortingManager(QObject *parent = nullptr);
    ~CodePortingManager();

    void resetUI();

    ConfigWidget *cfgWidget = nullptr;
    OutputPane *outputPane = nullptr;
    ReportPane *reportPane = nullptr;

    CodePorting codeporting;
};

#endif // CODEPORTINGMANAGER_H
