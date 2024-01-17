// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditormenu.h"
#include "gui/texteditor.h"

#include <QMenu>

namespace ActionId {
static constexpr char Copy[] { "copy" };
static constexpr char Cut[] { "cut" };
static constexpr char Paste[] { "paste" };
static constexpr char Delete[] { "delete" };
static constexpr char SelectAll[] { "select-all" };
static constexpr char Undo[] { "undo" };
static constexpr char Redo[] { "redo" };
}

TextEditorMenu::TextEditorMenu(QObject *parent)
    : AbstractEditorMenu(parent)
{
    predicateName[ActionId::Copy] = tr("Copy");
    predicateName[ActionId::Cut] = tr("Cut");
    predicateName[ActionId::Paste] = tr("Paste");
    predicateName[ActionId::Delete] = tr("Delete");
    predicateName[ActionId::SelectAll] = tr("Select All");
    predicateName[ActionId::Undo] = tr("Undo");
    predicateName[ActionId::Redo] = tr("Redo");
}

QString TextEditorMenu::name() const
{
    return "EditorMenu";
}

bool TextEditorMenu::initialize(const QVariantHash &params)
{
    // TODO
    return false;
}

bool TextEditorMenu::create(QMenu *parent)
{
    QAction *action { nullptr };
    if (!editor->isReadOnly()) {
        action = parent->addAction(predicateName[ActionId::Undo], this, [this] { editor->undo(); });
        action->setEnabled(editor->isUndoAvailable());
        predicateAction[ActionId::Undo] = action;

        action = parent->addAction(predicateName[ActionId::Redo], this, [this] { editor->redo(); });
        action->setEnabled(editor->isRedoAvailable());
        predicateAction[ActionId::Redo] = action;

        parent->addSeparator();

        action = parent->addAction(predicateName[ActionId::Cut], this, [this] { editor->cut(); });
        action->setEnabled(editor->hasSelectedText());
        predicateAction[ActionId::Cut] = action;
    }

    action = parent->addAction(predicateName[ActionId::Copy], this, [this] { editor->copy(); });
    action->setEnabled(editor->hasSelectedText());
    predicateAction[ActionId::Copy] = action;

    if (!editor->isReadOnly()) {
        action = parent->addAction(predicateName[ActionId::Paste], this, [this] { editor->paste(); });
        action->setEnabled(editor->SendScintilla(TextEditor::SCI_CANPASTE));
        predicateAction[ActionId::Paste] = action;

        action = parent->addAction(predicateName[ActionId::Delete], this, [this] { editor->SendScintilla(TextEditor::SCI_CLEAR); });
        action->setEnabled(editor->hasSelectedText());
        predicateAction[ActionId::Delete] = action;
    }

    parent->addSeparator();
    action = parent->addAction(predicateName[ActionId::SelectAll], this, [this] { editor->selectAll(); });
    action->setEnabled(editor->length() != 0);
    predicateAction[ActionId::SelectAll] = action;

    return AbstractEditorMenu::create(parent);
}

void TextEditorMenu::updateState(QMenu *parent)
{
    AbstractEditorMenu::updateState(parent);
}

bool TextEditorMenu::triggered(QAction *action)
{
    return AbstractEditorMenu::triggered(action);
}

AbstractEditorMenu *TextEditorMenu::menu(QAction *action) const
{
    if (!action)
        return nullptr;

    if (predicateAction.values().contains(action))
        return const_cast<TextEditorMenu *>(this);

    return AbstractEditorMenu::menu(action);
}
