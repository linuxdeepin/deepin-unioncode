// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include <QWidget>
#include <QSplitter>

class WorkspaceWidgetPrivate;
class WorkspaceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WorkspaceWidget(QWidget *parent = nullptr);

    QString selectedText() const;
    QString cursorBeforeText() const;
    QString cursorBehindText() const;
    QStringList modifiedFiles() const;
    void saveAll() const;
    void replaceSelectedText(const QString &text);
    void showTips(const QString &tips);
    void insertText(const QString &text);
    void undo();

private:
    QSharedPointer<WorkspaceWidgetPrivate> d { nullptr };
};

#endif   // WORKSPACEWIDGET_H
