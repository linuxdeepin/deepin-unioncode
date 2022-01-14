/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "editfilestatusbar.h"
#include <QMap>
#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>

class EditFileStatusBarPrivate
{
    friend class EditFileStatusBar;
    QLabel infoLabel;
    QColor background;
    QMap<EditFileStatusBar::StandardButton, QPushButton *> buttons;
};

EditFileStatusBar::EditFileStatusBar(QWidget *parent)
    : QStatusBar(parent), d(new EditFileStatusBarPrivate)
{
}

EditFileStatusBar::~EditFileStatusBar()
{
    if (d) {
        for (auto button : d->buttons.values()) {
            delete button;
        }
        d->buttons.clear();
        delete d;
    }
}

QPushButton *EditFileStatusBar::button(EditFileStatusBar::StandardButton button)
{
    return d->buttons[button];
}

EditFileStatusBar *EditFileStatusBar::changedReload(const QString &filePath)
{
    auto self = new EditFileStatusBar();
    self->d->infoLabel.setText(
            tr("File Path: %0").arg(filePath)
            + "\n"
            + tr("The current file has changed. Do you want to reload the current file?"));
    self->d->buttons[StandardButton::Reload] = new QPushButton("Reload");
    self->d->buttons[StandardButton::Cancel] = new QPushButton("Cancel");
    self->d->background = QColor(0xff0000);
    self->setAutoFillBackground(true);

    self->addWidget(&(self->d->infoLabel), 0);
    for (auto button : self->d->buttons) {
        button->setFixedSize(60, 20);
        self->addWidget(button);
    }

    QPalette palette;
    palette.setBrush(QPalette::ColorRole::Background, QBrush(self->d->background));
    self->setPalette(palette);
    return self;
}
