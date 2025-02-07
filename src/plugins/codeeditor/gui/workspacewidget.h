// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include "services/editor/editor_define.h"

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
    QString rangeText(const QString &fileName, const dpfservice::Edit::Range &range);
    dpfservice::Edit::Range codeRange(const QString &fileName, const dpfservice::Edit::Position &pos);
    dpfservice::Edit::Range selectionRange(const QString &fileName);
    void setText(const QString &text);
    void saveAll() const;
    void saveAs(const QString &from, const QString &to = "");
    void replaceSelectedText(const QString &text);
    void showTips(const QString &tips);
    void insertText(const QString &text);
    void undo();
    void reloadFile(const QString &fileName);
    void setFileModified(const QString &fileName, bool isModified);
    QStringList openedFiles() const;
    QString fileText(const QString &fileName) const;
    void replaceAll(const QString &fileName, const QString &oldText, const QString &newText, bool caseSensitive, bool wholeWords);
    void replaceText(const QString &fileName, int line, int index, int length, const QString &after);
    void replaceRange(const QString &fileName, const dpfservice::Edit::Range &range, const QString &newText);
    TabWidget *currentTabWidget() const;
    dpfservice::Edit::Position cursorPosition();
    QString lineText(const QString &fileName, int line);

    void eOLAnnotate(const QString &fileName, const QString &title, const QString &contents, int line, int type);
    void clearEOLAnnotation(const QString &file, const QString &title);
    void clearAllEOLAnnotation(const QString &title);
    void annotate(const QString &file, const QString &title, const QString &contents, int line, int type);
    void clearAnnotation(const QString &file, const QString &title);
    void clearAllAnnotation(const QString &title);

    void registerWidget(const QString &id, AbstractEditWidget *widget);
    void switchWidget(const QString &id);
    void switchDefaultWidget();

    int backgroundMarkerDefine(const QString &fileName, const QColor &color, int defaultMarker);
    void setRangeBackgroundColor(const QString &fileName, int startLine, int endLine, int marker);
    dpfservice::Edit::Range getBackgroundRange(const QString &fileName, int marker);
    void clearAllBackgroundColor(const QString &fileName, int marker);
    void showLineWidget(int line, QWidget *widget);
    void closeLineWidget();

protected:
    bool event(QEvent *event) override;

private:
    QSharedPointer<WorkspaceWidgetPrivate> d { nullptr };
};

#endif   // WORKSPACEWIDGET_H
