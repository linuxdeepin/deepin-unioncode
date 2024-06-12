// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorlabel.h"

#include <QLabel>
#include <QHBoxLayout>

EditorLabel::EditorLabel(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    cursorlabel = new QLabel(this);
    layout->addWidget(cursorlabel);
}

void EditorLabel::updateCursor(int line, int column)
{
    QString format(tr("Line %1 Column %2"));
    cursorlabel->setText(format.arg(line + 1).arg(column + 1));
}
