// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutedit.h"

ShortCutEdit::ShortCutEdit(QWidget *parent)
    : DKeySequenceEdit(parent)
{
    menu = new DMenu(this);
    QAction *clearAction = menu->addAction(tr("Clear"));
    connect(clearAction, &QAction::triggered, this, &ShortCutEdit::clearText);
}

void ShortCutEdit::clearText() 
{
    this->clear();
    emit shortcutCleared();
}