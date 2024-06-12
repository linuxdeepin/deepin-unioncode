// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "statusinfomanager.h"
#include "editorlabel.h"
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
    void updateLabelInfo();
    void handleFocusChanged(QWidget *old, QWidget *now);

public:
    TextEditor *currentEditor { nullptr };
    EditorLabel *editorLabel { nullptr };
};

void StatusInfoManagerPrivate::init(WindowService *winSrv)
{
    editorLabel = new EditorLabel();
    winSrv->addStatusBarItem(editorLabel);
}

void StatusInfoManagerPrivate::initConnect()
{
    connect(qApp, &QApplication::focusChanged, this, &StatusInfoManagerPrivate::handleFocusChanged);
}

void StatusInfoManagerPrivate::resetEditor(TextEditor *edit)
{
    if (currentEditor == edit)
        return;

    if (currentEditor)
        disconnect(currentEditor, &TextEditor::cursorPositionChanged, editorLabel, &EditorLabel::updateCursor);

    currentEditor = edit;
    connect(currentEditor, &TextEditor::cursorPositionChanged, editorLabel, &EditorLabel::updateCursor);
    updateLabelInfo();
}

void StatusInfoManagerPrivate::updateLabelInfo()
{
    int line = 0, col = 0;
    currentEditor->getCursorPosition(&line, &col);
    editorLabel->updateCursor(line, col);
}

void StatusInfoManagerPrivate::handleFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)

    auto edit = qobject_cast<TextEditor *>(now);
    if (!edit)
        return;

    resetEditor(edit);
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
