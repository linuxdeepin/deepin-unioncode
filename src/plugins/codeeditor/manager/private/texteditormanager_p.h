// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITORMANAGER_P_H
#define TEXTEDITORMANAGER_P_H

#include "manager/texteditormanager.h"

class TextEditorManagerPrivate
{
public:
    explicit TextEditorManagerPrivate(TextEditorManager *qq);

public:
    TextEditorManager *q;

    QHash<QString, TextEditor *> editorHash;
};

#endif   // TEXTEDITORMANAGER_P_H
