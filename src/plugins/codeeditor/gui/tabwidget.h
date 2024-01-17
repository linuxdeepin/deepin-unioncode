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
    void replaceSelectedText(const QString &text);

    void setEditorCursorPosition(int pos);
    int editorCursorPosition();
    void setEditorScrollValue(int value);
    int editorScrollValue();

public slots:
    void openFile(const QString &fileName);

signals:
    void closeRequested();
    void splitRequested(Qt::Orientation ori, const QString &fileName);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    std::once_flag flag;
    QSharedPointer<TabWidgetPrivate> d { nullptr };
};

#endif   // TABWIDGET_H
