// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include "services/editor/editor_define.h"

#include <QWidget>

class TextEditor;
class QsciDocument;
class TabWidgetPrivate;
class TabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);

    void setCloseButtonVisible(bool visible);
    void setSplitButtonVisible(bool visible);

    QString currentFile() const;
    QString currentDocumentContent() const;
    QString selectedText() const;
    QString cursorBeforeText() const;
    QString cursorBehindText() const;
    QStringList modifiedFiles() const;
    QStringList openedFiles() const;
    QString rangeText(const QString &fileName, const dpfservice::Edit::Range &range, bool &found);
    dpfservice::Edit::Range selectionRange(const QString &fileName, bool &found);
    void setText(const QString &text);
    QString fileText(const QString &fileName, bool *success = nullptr);
    void replaceAll(const QString &fileName, const QString &oldText,
                    const QString &newText, bool caseSensitive, bool wholeWords);
    void replaceText(const QString &fileName, int line, int index, int length, const QString &after);
    bool replaceRange(const QString &fileName, const dpfservice::Edit::Range &range, const QString &newText);
    void saveAll() const;
    bool saveAs(const QString &from, const QString &to);
    void reloadFile(const QString &fileName);
    void setFileModified(const QString &fileName, bool isModified);
    void closeFileEditor(const QString &fileName);
    void closeFileEditor();
    void closeAllEditor();
    void switchHeaderSource();
    void followSymbolUnderCursor();
    void findUsage();
    void renameSymbol();
    Q_INVOKABLE void replaceSelectedText(const QString &text);
    Q_INVOKABLE void showTips(const QString &tips);
    Q_INVOKABLE void insertText(const QString &text);
    Q_INVOKABLE void undo();
    void gotoNextPosition();
    void gotoPreviousPosition();
    QString lineText(const QString &fileName, int line);

    bool eOLAnnotate(const QString &fileName, const QString &title, const QString &contents, int line, int type);
    bool clearEOLAnnotation(const QString &fileName, const QString &title);
    void clearAllEOLAnnotation(const QString &title);
    bool annotate(const QString &fileName, const QString &title, const QString &contents, int line, int type);
    bool clearAnnotation(const QString &fileName, const QString &title);
    void clearAllAnnotation(const QString &title);

    void setEditorCursorPosition(int pos);
    int editorCursorPosition();
    void setEditorScrollValue(int value);
    int editorScrollValue();

    void addBreakpoint(const QString &fileName, int line, bool enabled);
    void removeBreakpoint(const QString &fileName, int line);
    void setBreakpointEnabled(const QString &fileName, int line, bool enabled);
    void toggleBreakpoint();
    void clearAllBreakpoints();
    void handleSetComment();
    void handleShowOpenedFiles(const int &x, const int &y, const QSize &size);

    int zoomValue();
    void updateZoomValue(int value);

    int backgroundMarkerDefine(const QString &fileName, const QColor &color, int defaultMarker);
    bool setRangeBackgroundColor(const QString &fileName, int startLine, int endLine, int marker);
    dpfservice::Edit::Range getBackgroundRange(const QString &fileName, int marker, bool &found);
    bool clearAllBackground(const QString &fileName, int marker);
    void showLineWidget(int line, QWidget *widget);
    void closeLineWidget();
    void cursorPosition(int *line, int *index);

    TextEditor *currentEditor() const;
    TextEditor *findEditor(const QString &fileName);

public slots:
    void openFile(const QString &fileName, QsciDocument *doc = nullptr);
    void setDebugLine(int line);
    void removeDebugLine();
    void gotoLine(int line);
    void gotoPosition(int line, int column);
    void saveFile(const QString &fileName);

signals:
    void closeRequested();
    void splitRequested(Qt::Orientation ori, const QString &fileName);
    void zoomValueChanged();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QSharedPointer<TabWidgetPrivate> d { nullptr };
};

#endif   // TABWIDGET_H
