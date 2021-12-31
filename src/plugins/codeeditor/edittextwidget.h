/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef EDITTEXTWIDGET_H
#define EDITTEXTWIDGET_H

#include <QTextEdit>

#include "ScintillaEdit.h"
#include "common/lsp/protocol.h"

class EditTextWidgetPrivate;
class EditTextWidget : public ScintillaEdit
{
    Q_OBJECT
    EditTextWidgetPrivate *const d;
public:
    explicit EditTextWidget(QWidget * parent = nullptr);
    virtual ~EditTextWidget();
    QString currentFile();

public slots:
    void setCurrentFile(const QString &filePath, const QString &workspaceFolder);
    void publishDiagnostics(const lsp::Protocol::Diagnostics &diagnostics);
    void debugMarginClieced(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin);
};

#endif // EDITTEXTWIDGET_H
