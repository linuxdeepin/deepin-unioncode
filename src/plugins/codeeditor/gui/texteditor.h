// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <Qsci/qsciscintilla.h>

class TextEditorPrivate;
class TextEditor : public QsciScintilla
{
    Q_OBJECT
public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor() override;

    virtual void setFile(const QString &fileName);
    QString getFile() const;

    void save();
    void saveAs();
    void reload();

    // debug
    void addBreakpoint(int line);
    void removeBreakpoint(int line);
    void setBreakpointEnabled(int line);
    bool hasBreakpoint(int line);
    void gotoNextBreakpoint();
    void gotoPreviousBreakpoint();
    void clearAllBreakpoints();

    void setDebugLine(int line);
    void removeDebugLine();

    // bookmark
    void addBookmark(int line);
    void removeBookmark(int line);
    bool hasBookmark(int line);
    void gotoNextBookmark();
    void gotoPreviousBookmark();
    void clearAllBookmarks();

    // util
    int currentLineNumber();
    void gotoLine(int line);
    void gotoPosition(int pos);
    int cursorPosition();
    void setLineBackgroundColor(int line, const QColor &color);
    void resetLineBackgroundColor(int line);
    void clearLineBackgroundColor();
    void showTips(const QString &tips);
    void addAnnotation(const QString &title, const QString &content, int line, int type);
    void removeAnnotation(const QString &title);

    QString cursorBeforeText() const;
    QString cursorBehindText() const;

public slots:
    void onMarginClicked(int margin, int line, Qt::KeyboardModifiers state);
    void updateLineNumberWidth(bool isDynamicWidth);
    void onScrollValueChanged(int value);
    void onCursorPositionChanged(int line, int index);

signals:
    void fileSaved(const QString &fileName);
    void fileClosed(const QString &fileName);
    void textAdded(int pos, int len, int added, const QString &text, int line);
    void textRemoved(int pos, int len, int removed, const QString &text, int line);
    void zoomValueChanged();
    void documentHovered(int pos);
    void documentHoveredWithCtrl(int pos);
    void documentHoverEnd(int pos);

private:
    void init();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QSharedPointer<TextEditorPrivate> d { nullptr };
};
#endif   // TEXTEDITOR_H
