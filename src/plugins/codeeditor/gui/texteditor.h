// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "common/lsp/protocol/new/languagefeatures.h"

#include <Qsci/qsciscintilla.h>

enum CommentSettings {
    Line = 0,
    BlockStart,
    BlockEnd
};

class CodeCompletionWidget;
class LanguageClientHandler;
class TextEditorPrivate;
class TextEditor : public QsciScintilla
{
    Q_OBJECT
public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor() override;

    void openFile(const QString &fileName);
    void openFileWithDocument(const QString &fileName, const QsciDocument &doc);
    QString getFile() const;
    QString documentEncode() const;

    void save();
    void saveAs();
    void saveAs(const QString &fileName);
    void reload();
    bool reload(const QString &encode);

    // debug
    void addBreakpoint(int line, bool enabled = true);
    void removeBreakpoint(int line);
    void toggleBreakpoint();
    void setBreakpointEnabled(int line, bool enabled);
    void setBreakpointCondition(int line);
    bool breakpointEnabled(int line);
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

    intptr_t searchInTarget(QByteArray &text2Find, size_t fromPos, size_t toPos) const;
    intptr_t replaceTarget(QByteArray &str2replace, intptr_t fromTargetPos = -1, intptr_t toTargetPos = -1) const;

    // util
    int currentLineNumber();
    void gotoLine(int line);
    void gotoPosition(int pos);
    int cursorLastPosition();
    int cursorPosition();
    int backgroundMarkerDefine(const QColor &color, int defaultMarker);
    void setRangeBackgroundColor(int startLine, int endLine, int marker);
    void getBackgroundRange(int marker, int *startLine, int *endLine);
    void clearAllBackgroundColor(int marker);
    void showTips(const QString &tips);
    void showTips(int pos, const QString &tips);
    void showTips(int pos, QWidget *w);
    void cancelTips();
    void addAnnotation(const QString &title, const QString &content, int line, int type);
    void addAnnotation(const QString &content, int line, int type);
    void removeAnnotation(const QString &title);
    void addEOLAnnotation(const QString &title, const QString &content, int line, int type);
    void removeEOLAnnotation(const QString &title);
    void commentOperation();
    QString getFileType();
    QStringList getFileCommentSettings(const QMap<QString, QVariant> &commentSettings);
    QString addCommentPrefix(const QString &selectedTexts, const QString &commentSymbol);
    QString delCommentPrefix(const QString &selectedTexts, const QString &commentSymbol);
    bool hasUncommentedLines(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo, const QStringList &settings);
    void addCommentToSelectedLines(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo, const QStringList &settings);
    void delCommentToSelectedLines(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo, const QStringList &settings);
    bool selectionStatus(const int &lineFrom, const int &lineTo, const int &indexFrom, const int &indexTo);
    QPoint pointFromPosition(int position);
    int positionFromPoint(int x, int y);
    void replaceRange(int lineFrom, int indexFrom, int lineTo, int indexTo, const QString &text, bool changePos = false);
    void replaceRange(int startPosition, int endPosition, const QString &text, bool changePos = false);
    void insertText(const QString &text);
    LanguageClientHandler *languageClient() const;
    int wordStartPositoin(int position);
    int wordEndPosition(int position);
    void switchHeaderSource();
    void followSymbolUnderCursor();
    void findUsage();
    void renameSymbol();

    QString cursorBeforeText() const;
    QString cursorBehindText() const;

    void setAutomaticInvocationEnabled(bool enabled);
    bool isAutomaticInvocationEnabled() const;
    bool showLineWidget(int line, QWidget *widget);
    void closeLineWidget();

public slots:
    void onMarginClicked(int margin, int line, Qt::KeyboardModifiers state);
    void updateLineNumberWidth(bool isDynamicWidth);
    void onScrollValueChanged(int value);
    void onCursorPositionChanged(int line, int index);

protected:
    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual bool event(QEvent *event) override;

signals:
    void fileClosed(const QString &fileName);
    void textAdded(int pos, int len, int added, const QString &text, int line);
    void textRemoved(int pos, int len, int removed, const QString &text, int line);
    void zoomValueChanged();
    void documentHovered(int pos);
    void documentHoverEnd(int pos);
    void requestFollowType(int pos);
    void followTypeEnd();
    void contextMenuRequested(QMenu *menu);
    void focusOut();
    void cursorRecordChanged(int pos);
    void requestOpenFiles(const QList<QUrl> &fileList);
    void delayCursorPositionChanged(int line, int index);
    void cursorModeChanged();

private:
    void init();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    TextEditorPrivate *d { nullptr };
};
#endif   // TEXTEDITOR_H
