// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include <QWidget>
#include <QSplitter>

class AbstractEditWidget;
class WorkspaceWidgetPrivate;
class WorkspaceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WorkspaceWidget(QWidget *parent = nullptr);

    QString currentFile() const;
    QString currentDocumentContent() const;
    QString selectedText() const;
    QString cursorBeforeText() const;
    QString cursorBehindText() const;
    QStringList modifiedFiles() const;
    void setText(const QString &text);
    void saveAll() const;
    void saveAs(const QString &from, const QString &to = "");
    void replaceSelectedText(const QString &text);
    void showTips(const QString &tips);
    void setCompletion(const QString &info, const QIcon &icon, const QKeySequence &key);
    void insertText(const QString &text);
    void undo();
    void reloadFile(const QString &fileName);
    void setFileModified(const QString &fileName, bool isModified);
    void closeFileEditor(const QString &fileName);
    QStringList openedFiles() const;
    QString fileText(const QString &fileName) const;

    void registerWidget(const QString &id, AbstractEditWidget *widget);
    void switchWidget(const QString &id);
    void switchDefaultWidget();

protected:
    bool event(QEvent *event) override;

private:
    QSharedPointer<WorkspaceWidgetPrivate> d { nullptr };
};

#endif   // WORKSPACEWIDGET_H
