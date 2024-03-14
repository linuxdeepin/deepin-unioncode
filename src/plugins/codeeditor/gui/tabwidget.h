// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QWidget>
#include <mutex>

class TabWidgetPrivate;
class TabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);

    void setCloseButtonVisible(bool visible);
    void setSplitButtonVisible(bool visible);

    QString selectedText() const;
    QString cursorBeforeText() const;
    QString cursorBehindText() const;
    QStringList modifiedFiles() const;
    QStringList openedFiles() const;
    void saveAll() const;
    bool saveAs(const QString &from, const QString &to);
    void reloadFile(const QString &fileName);
    void setFileModified(const QString &fileName, bool isModified);
    void closeFileEditor(const QString &fileName);
    Q_INVOKABLE void replaceSelectedText(const QString &text);
    Q_INVOKABLE void showTips(const QString &tips);
    Q_INVOKABLE void insertText(const QString &text);
    Q_INVOKABLE void undo();
    void gotoNextPosition();
    void gotoPreviousPosition();
    bool checkAndResetSaveState(const QString &fileName);

    void setEditorCursorPosition(int pos);
    int editorCursorPosition();
    void setEditorScrollValue(int value);
    int editorScrollValue();

    void addBreakpoint(const QString &fileName, int line);
    void removeBreakpoint(const QString &fileName, int line);
    void clearAllBreakpoints();

    int zoomValue();
    void setZoomValue(int value);
    void updateZoomValue(int value);

public slots:
    void openFile(const QString &fileName);
    void setDebugLine(int line);
    void removeDebugLine();
    void gotoLine(int line);
    void gotoPosition(int line, int column);

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
