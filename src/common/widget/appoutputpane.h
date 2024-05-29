// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPOUTPUTPANE_H
#define APPOUTPUTPANE_H

#include "outputpane.h"

#include <DWidget>
#include <DFrame>

#include <QPlainTextEdit>

#include <functional>

class OutputWindowPrivate;
class AppOutputPane : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    AppOutputPane(QWidget *parent = nullptr);
    ~AppOutputPane() override;

    static AppOutputPane *instance();
    OutputPane *defaultPane();

    Q_INVOKABLE void createApplicationPane(const QString &id, const QString &program);
    OutputPane *getOutputPaneById(const QString &id);
    void appendTextToApplication(const QString &id,
                                 const QString &text,
                                 OutputPane::OutputFormat format,
                                 OutputPane::AppendMode mode);

    void setProcessFinished(const QString &id);

    using StopHandler = std::function<void()>;
    void setStopHandler(const QString &id, StopHandler handler);

    void registerItemToToolBar(const QString& toolbarName, QAction *abstractAction, bool addSeparator);
    void bindToolBarToPane(const QString &toolbarName, OutputPane *pane);
signals:
    void paneCreated(const QString &id);

public slots:
    void slotCloseOutputPane();

private:
    void initUi();
    void initTabWidget();
    void stop(const QString &id);

    OutputWindowPrivate *d;
};

#endif   // APPOUTPUTPANE_H
