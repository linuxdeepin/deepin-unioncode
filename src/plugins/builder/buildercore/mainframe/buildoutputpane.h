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
#ifndef BUILDOUTPUTPANE_H
#define BUILDOUTPUTPANE_H

#include "base/abstractoutputpane.h"
#include "buildstep.h"

#include <QPlainTextEdit>

class OutputWindowPrivate;
class BuildOutputPane : public QPlainTextEdit
{
    Q_OBJECT
public:
    BuildOutputPane();
    ~BuildOutputPane() override;

    void clearContents();
    void appendText(const QString &text, const QTextCharFormat &format = QTextCharFormat());
    void appendText(const QString &text, OutputFormat format);

    bool isScrollbarAtBottom() const;
    QString doNewlineEnforcement(const QString &out);
    void scrollToBottom();
private:
    OutputWindowPrivate *d = nullptr;
};

#endif // BUILDOUTPUTPANE_H
