// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPOUTPUTPANE_H
#define APPOUTPUTPANE_H

#include "base/abstractoutputpane.h"
#include "debuggerglobals.h"

#include <QPlainTextEdit>

class OutputWindowPrivate;
class AppOutputPane : public QPlainTextEdit
{
    Q_OBJECT
public:
    AppOutputPane();
    ~AppOutputPane() override;

    void clearContents();

    bool isScrollbarAtBottom() const;
    QString doNewlineEnforcement(const QString &out);
    void scrollToBottom();

protected:
    void contextMenuEvent(QContextMenuEvent * event) override;

public slots:
    void appendText(const QString &text, OutputPane::OutputFormat format);

private:
    void appendText(const QString &text, const QTextCharFormat &format = QTextCharFormat());
    QList<QAction*> actionFactory();
    OutputWindowPrivate *d = nullptr;
};

#endif   // APPOUTPUTPANE_H
