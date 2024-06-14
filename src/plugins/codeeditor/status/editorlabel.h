// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORLABEL_H
#define EDITORLABEL_H

#include <QWidget>

class QLabel;

class EditorLabel : public QWidget
{
    Q_OBJECT
public:
    explicit EditorLabel(QWidget *parent = nullptr);

public Q_SLOTS:
    void updateCursor(int line, int column);

private:
    QLabel *cursorlabel { nullptr };
};

#endif   // EDITORLABEL_H
