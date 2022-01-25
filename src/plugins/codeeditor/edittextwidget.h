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
#include "common/common.h"

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
    void setCurrentFile(const QString &filePath);
    void debugPointAllDelete();
    void jumpToLine(int line);
    void runningToLine(int line);
    void runningEnd();

private slots:
    // lsp server called
    void publishDiagnostics(const lsp::Diagnostics &diagnostics);
    void tokenFullResult(const QList<lsp::Data> &tokens);
    void debugMarginClicked(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin);
    void tokenDefinitionsSave(const lsp::SemanticTokensProvider &provider);
    void completionsSave(const lsp::CompletionProvider &provider);
    void hoverRequest();
    void hoverMessage(const lsp::Hover &hover);
    void definitionSave(const lsp::DefinitionProvider &provider);

    // scintilla called
    void sciModified(Scintilla::ModificationFlags type, Scintilla::Position position,
                     Scintilla::Position length, Scintilla::Position linesAdded, const QByteArray &text, Scintilla::Position line,
                     Scintilla::FoldLevel foldNow, Scintilla::FoldLevel foldPrev);
    void dwellStartNotify(int x, int y);
    void dwellEndNotify(int x, int y);
    void setDefinitionWordStyle();
    void cleanDefinitionWordStyle();

private:
    void setDefaultStyle();
    bool setLspIndicStyle(const QString &languageID);

protected:
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void enterEvent(QEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
};

#endif // EDITTEXTWIDGET_H
