// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPOUTPUTPANE_H
#define APPOUTPUTPANE_H

#include "outputpane.h"

#include <DWidget>

#include <QPlainTextEdit>

class OutputWindowPrivate;
class AppOutputPane : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    AppOutputPane(QWidget *parent = nullptr);
    ~AppOutputPane() override;

    static AppOutputPane *instance();
    OutputPane *defaultPane();

    Q_INVOKABLE void createApplicationPane(const quint64 &pid, const QString &program);
    OutputPane *getOutputPaneByPid(const quint64 &pid);
    void appendTextToApplication(const quint64 &pid,
                                 const QString &text,
                                 OutputPane::OutputFormat format,
                                 OutputPane::AppendMode mode);

    void setProcessFinished(const quint64 &pid);

signals:
    void paneCreated(const quint64 &pid);

public slots:
    void slotCloseOutputPane();

private:
    void initUi();
    void initTabWidget();

    OutputWindowPrivate *d;
};

#endif   // APPOUTPUTPANE_H
