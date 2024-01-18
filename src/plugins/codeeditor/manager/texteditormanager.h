// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITORMANAGER_H
#define TEXTEDITORMANAGER_H

#include "gui/texteditor.h"

class TextEditorManagerPrivate;
class TextEditorManager : public QObject
{
    Q_OBJECT
public:
    explicit TextEditorManager(QObject *parent = nullptr);

    TextEditor *createEditor(QWidget *parent, const QString &fileName = "");
    TextEditor *findEditor(const QString &fileName);

    void gotoNextPosition();
    void gotoPreviousPsontion();
    void clearAllBreakpoints();

public slots:
    void onLinePositionChanged(int line, int pos);
    void onEditorDestroyed(const QString &fileName);

private:
    QSharedPointer<TextEditorManagerPrivate> d { nullptr };
};

#endif   // TEXTEDITORMANAGER_H
