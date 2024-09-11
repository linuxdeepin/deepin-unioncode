// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include <QWidget>
#include <QSplitter>

#include <functional>

class TabWidget;
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
    QStringList openedFiles() const;
    QString fileText(const QString &fileName) const;
    void replaceAll(const QString &fileName, const QString &oldText, const QString &newText, bool caseSensitive, bool wholeWords);
    void replaceRange(const QString &fileName, int line, int index, int length, const QString &after);
    TabWidget *currentTabWidget() const;
    void cursorPosition(int *line, int *index);

    void registerWidget(const QString &id, AbstractEditWidget *widget);
    void switchWidget(const QString &id);
    void switchDefaultWidget();

    int setRangeBackgroundColor(const QString &fileName, int startLine, int endLine, const QColor &color);
    void clearRangeBackgroundColor(const QString &fileName, int startLine, int endLine, int marker);
    void clearAllBackgroundColor(const QString &fileName, int marker);
    void showLineWidget(int line, QWidget *widget);
    void closeLineWidget();

    using RepairCallback = std::function<void(const QString &info)>;
    void registerDiagnosticRepairTool(const QString &toolName, RepairCallback callback);
    QMap<QString, RepairCallback> getDiagnosticRepairTool() const;

protected:
    bool event(QEvent *event) override;

private:
    QSharedPointer<WorkspaceWidgetPrivate> d { nullptr };
};

#endif   // WORKSPACEWIDGET_H
