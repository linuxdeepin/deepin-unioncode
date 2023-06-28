// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textedittitlebar.h"

#include <QMap>
#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

class TextEditTitleBarPrivate
{
    friend class TextEditTitleBar;
    QLabel infoLabel;
    QColor background;
    QMap<TextEditTitleBar::StandardButton, QPushButton *> buttons;
};

TextEditTitleBar::TextEditTitleBar(QWidget *parent)
    : QStatusBar(parent), d(new TextEditTitleBarPrivate)
{
}

TextEditTitleBar::~TextEditTitleBar()
{
    if (d) {
        for (auto button : d->buttons.values()) {
            delete button;
        }
        d->buttons.clear();
        delete d;
    }
}

QPushButton *TextEditTitleBar::button(TextEditTitleBar::StandardButton button)
{
    return d->buttons[button];
}

TextEditTitleBar *TextEditTitleBar::changedReload(const QString &filePath)
{
    auto self = new TextEditTitleBar();
    self->d->infoLabel.setText(
            QLabel::tr("File Path: %0").arg(filePath)
            + "\n"
            + QLabel::tr("The current file has changed. Do you want to reload the current file?"));
    self->d->buttons[StandardButton::Reload] = new QPushButton(QPushButton::tr("Reload"));
    self->d->buttons[StandardButton::Cancel] = new QPushButton(QPushButton::tr("Cancel"));
    self->d->background = QColor(0xff0000);
    self->setAutoFillBackground(true);

    connect(self->d->buttons[StandardButton::Reload], &QPushButton::clicked, [=](){
        self->reloadfile();
        self->close();
    });

    connect(self->d->buttons[StandardButton::Cancel], &QPushButton::clicked, [=](){
        self->close();
    });

    self->addWidget(&(self->d->infoLabel), 0);
    for (auto button : self->d->buttons) {
        button->setFixedSize(60, 30);
        self->addWidget(button);
    }

    QPalette palette;
    palette.setBrush(QPalette::ColorRole::Window, QBrush(self->d->background));
    self->setPalette(palette);
    return self;
}
