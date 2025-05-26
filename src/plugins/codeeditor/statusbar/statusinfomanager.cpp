// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "statusinfomanager.h"
#include "editorstatusbar.h"
#include "gui/texteditor.h"

#include "services/window/windowservice.h"

#include <QApplication>

using namespace dpfservice;

class StatusInfoManagerPrivate : public QObject
{
public:
    void init(WindowService *winSrv);
    void initConnect();

    void resetEditor(TextEditor *edit);
    void updateStatusBar();
    void handleFocusChanged(QWidget *old, QWidget *now);
    void handleEncodeChanged(const QString &encode);

    void updateEditorCursorMode();
    void updateEditorEncodedMode();

public:
    TextEditor *currentEditor { nullptr };
    EditorStatusBar *statusBar { nullptr };
};

void StatusInfoManagerPrivate::init(WindowService *winSrv)
{
    statusBar = new EditorStatusBar();
    statusBar->setVisible(false);
    winSrv->addStatusBarItem(statusBar);
}

void StatusInfoManagerPrivate::initConnect()
{
    connect(qApp, &QApplication::focusChanged, this, &StatusInfoManagerPrivate::handleFocusChanged);
    connect(statusBar, &EditorStatusBar::encodeChanged, this, &StatusInfoManagerPrivate::handleEncodeChanged);
}

void StatusInfoManagerPrivate::resetEditor(TextEditor *edit)
{
    if (currentEditor == edit)
        return;

    if (currentEditor) {
        currentEditor->disconnect(this);
        currentEditor->disconnect(statusBar);
    }

    currentEditor = edit;
    statusBar->setVisible(true);
    connect(edit, &TextEditor::destroyed, this, [this] {
        statusBar->setVisible(false);
        currentEditor = nullptr;
    });
    connect(currentEditor, &TextEditor::cursorPositionChanged, statusBar, &EditorStatusBar::updateCursor);
    connect(currentEditor, &TextEditor::cursorModeChanged, this, &StatusInfoManagerPrivate::updateEditorCursorMode);
    updateStatusBar();
}

void StatusInfoManagerPrivate::updateStatusBar()
{
    int line = 0, col = 0;
    currentEditor->getCursorPosition(&line, &col);
    statusBar->updateCursor(line, col);

    updateEditorCursorMode();
    updateEditorEncodedMode();
}

void StatusInfoManagerPrivate::handleFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)

    auto edit = qobject_cast<TextEditor *>(now);
    if (!edit)
        return;

    resetEditor(edit);
}

void StatusInfoManagerPrivate::handleEncodeChanged(const QString &encode)
{
    if (!currentEditor)
        return;

    currentEditor->reload(encode);
    updateEditorEncodedMode();
}

void StatusInfoManagerPrivate::updateEditorCursorMode()
{
    if (currentEditor->isReadOnly()) {
        statusBar->updateCursorMode(EditorStatusBar::Readonly);
    } else {
        bool overwrite = currentEditor->overwriteMode();
        statusBar->updateCursorMode(overwrite ? EditorStatusBar::Overwrite : EditorStatusBar::Insert);
    }
}

void StatusInfoManagerPrivate::updateEditorEncodedMode()
{
    statusBar->updateEncodedMode(currentEditor->documentEncode());
}

StatusInfoManager::StatusInfoManager(QObject *parent)
    : QObject(parent),
      d(new StatusInfoManagerPrivate)
{
}

StatusInfoManager::~StatusInfoManager()
{
    delete d;
}

StatusInfoManager *StatusInfoManager::instance()
{
    static StatusInfoManager ins;
    return &ins;
}

void StatusInfoManager::init(WindowService *winSrv)
{
    d->init(winSrv);
    d->initConnect();
}
