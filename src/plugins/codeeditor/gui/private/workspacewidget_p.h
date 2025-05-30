// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_P_H
#define WORKSPACEWIDGET_P_H

#include "gui/workspacewidget.h"
#include "gui/tabwidget.h"

#include "base/abstracteditwidget.h"
#include "common/util/eventdefinitions.h"
#include "services/window/windowservice.h"
#include "services/session/sessionservice.h"
#include "Qsci/qscicommand.h"

#include <QStackedWidget>

class WorkspaceWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit WorkspaceWidgetPrivate(WorkspaceWidget *qq);

    void initUI();
    void initConnection();
    void initActions();
    void connectTabWidgetSignals(TabWidget *tabWidget);

    TabWidget *currentTabWidget() const;
    void doSplit(TabWidget *fromTW, QSplitter *spliter, int index, const QString &fileName);

    int showFileChangedConfirmDialog(const QString &fileName);
    int showFileRemovedConfirmDialog(const QString &fileName);

    void handleFileChanged();
    void handleFileRemoved();

public slots:
    void checkFileState();
    void onSplitRequested(Qt::Orientation ori, const QString &fileName);
    void onCloseRequested();
    void onFocusChanged(QWidget *old, QWidget *now);
    void onZoomValueChanged();
    void onFileDeleted(const QString &fileName);
    void onFileModified(const QString &fileName);

    void handleOpenFile(const QString &workspace, const QString &fileName);
    void handleCloseFile(const QString &fileName);
    void handleAddBreakpoint(const QString &fileName, int line, bool enabled);
    void handleRemoveBreakpoint(const QString &fileName, int line);
    void handleSetBreakpointEnabled(const QString &fileName, int line, bool enabled);
    void handleToggleBreakpoint();
    void handleBack();
    void handleForward();
    void handleSetDebugLine(const QString &fileName, int line);
    void handleRemoveDebugLine();
    void handleGotoLine(const QString &fileName, int line);
    void handleGotoPosition(const QString &fileName, int line, int column);
    void handleCloseCurrentEditor();
    void handleSwitchHeaderSource();
    void handleFollowSymbolUnderCursor();
    void handleFindUsage();
    void handleRenameSymbol();
    void handleSetModifiedAutoReload(const QString &fileName, bool flag);
    void handleSetComment();
    void handleShowOpenedFiles();
    void handleSaveSession();
    void handleSessionLoaded();

public:
    WorkspaceWidget *q;

    TabWidget *focusTabWidget { nullptr };
    QList<TabWidget *> tabWidgetList;
    QStackedWidget *stackWidget { nullptr };
    QMap<QString, AbstractEditWidget *> registeredWidget;

    QStringList autoReloadList;
    QStringList modifiedFileList;
    QStringList removedFileList;
    QTimer fileCheckTimer;

    dpfservice::SessionService *sessionSrv { nullptr };
};

#endif   // WORKSPACEWIDGET_P_H
