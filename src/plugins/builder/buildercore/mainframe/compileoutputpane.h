/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#ifndef COMPILEOUTPUTPANE_H
#define COMPILEOUTPUTPANE_H

#include "services/builder/builderglobals.h"

#include "base/abstractoutputpane.h"

#include <QPlainTextEdit>

class CompileOutputPanePrivate;
class CompileOutputPane : public QPlainTextEdit
{
    Q_OBJECT
public:
    CompileOutputPane(QWidget *parent = nullptr);
    ~CompileOutputPane() override;

    void clearContents();
    void appendText(const QString &text, const QTextCharFormat &format = QTextCharFormat());
    void appendText(const QString &text, OutputFormat format);

    bool isScrollbarAtBottom() const;
    QString doNewlineEnforcement(const QString &out);
    void scrollToBottom();
private:
    QString normalizeNewlines(const QString &text);

    CompileOutputPanePrivate *d = nullptr;
};

#endif // COMPILEOUTPUTPANE_H
