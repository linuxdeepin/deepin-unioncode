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
    void appendText(const QString &text, OutputFormat format);

private:
    void appendText(const QString &text, const QTextCharFormat &format = QTextCharFormat());
    QList<QAction*> actionFactory();
    OutputWindowPrivate *d = nullptr;
};

#endif   // APPOUTPUTPANE_H
