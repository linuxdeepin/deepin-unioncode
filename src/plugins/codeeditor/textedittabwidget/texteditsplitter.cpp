/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
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
#include "texteditsplitter.h"
#include "transceiver/codeeditorreceiver.h"

#include <QBoxLayout>
#include <QDebug>

static TextEditSplitter *ins{nullptr};

TextEditSplitter::TextEditSplitter(QWidget *parent)
    :QWidget(parent)
    , vLayout(new QVBoxLayout)
    , mainSplitter(new QSplitter)
{
    tabWidget = new TextEditTabWidget(mainSplitter);
    mainSplitter->addWidget(tabWidget);
    tabWidgets.append(tabWidget);
    newSplitters.append(mainSplitter);
    ++count;

    QObject::connect(EditorCallProxy::instance(),
                     QOverload<const newlsp::ProjectKey &, const QString &>::of(&EditorCallProxy::toOpenFile),
                     tabWidget, QOverload<const newlsp::ProjectKey &, const QString &>::of(&TextEditTabWidget::openFile));
    QObject::connect(tabWidget, &TextEditTabWidget::signalEditSplit, this, &TextEditSplitter::editSplit);

    QObject::connect(tabWidget, &TextEditTabWidget::signalEditClose, this, &TextEditSplitter::closeSplit);
    QObject::connect(tabWidget, &TextEditTabWidget::signalFocusInChange, this, &TextEditSplitter::doSelectedTextEditWidget);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch(20);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(mainSplitter);
    this->setLayout(vLayout);

}

TextEditSplitter::~TextEditSplitter()
{

}

void TextEditSplitter::editSplit(Qt::Orientation orientation, const QString &file)
{
    auto textEditTabWidget = qobject_cast<TextEditTabWidget*>(sender());
    auto splitter = qobject_cast<QSplitter *>(textEditTabWidget->parent());

    QSplitter *newSplitter = new QSplitter(splitter);
    newSplitters.append(newSplitter);
    newSplitter->setOrientation(orientation);
    newSplitter->setHandleWidth(0);
    textEditTabWidget->setParent(newSplitter);

    TextEditTabWidget *newTextEdit = new TextEditTabWidget(newSplitter);
    tabWidgets.append(newTextEdit);
    ++count;
    newTextEdit->openFile(file);
    newTextEdit->setFocus();
    newTextEdit->activateWindow();
    newSplitter->insertWidget(0, textEditTabWidget);
    newSplitter->insertWidget(1, newTextEdit);
    splitter->replaceWidget(0, newSplitter);

    // cancel all open file sig-slot from texteditwidget
    disconnect(EditorCallProxy::instance(),
               QOverload<const newlsp::ProjectKey &, const QString &>::of(&EditorCallProxy::toOpenFile),
               textEditTabWidget, QOverload<const newlsp::ProjectKey &, const QString &>::of(&TextEditTabWidget::openFile));

    // connect new texteditwidget openfile slot
    QObject::connect(EditorCallProxy::instance(),
                     QOverload<const newlsp::ProjectKey &, const QString &>::of(&EditorCallProxy::toOpenFile),
                     newTextEdit, QOverload<const newlsp::ProjectKey &, const QString &>::of(&TextEditTabWidget::openFile));

    // connect selected texteditwidget sig-slot bind, from texteditwidget focus
    QObject::connect(newTextEdit, &TextEditTabWidget::signalEditSplit, this, &TextEditSplitter::editSplit);
    QObject::connect(newTextEdit, &TextEditTabWidget::signalFocusInChange, this, &TextEditSplitter::doSelectedTextEditWidget);
    QObject::connect(newTextEdit, &TextEditTabWidget::signalEditClose, this, &TextEditSplitter::closeSplit);
}

void TextEditSplitter::doSelectedTextEditWidget()
{
    for (int i = 0; i < tabWidgets.size(); i++) {
        disconnect(EditorCallProxy::instance(),
                   QOverload<const newlsp::ProjectKey &, const QString &>::of(&EditorCallProxy::toOpenFile),
                   tabWidgets.at(i), QOverload<const newlsp::ProjectKey &, const QString &>::of(&TextEditTabWidget::openFile));
    }

    auto textEditTabWidget = qobject_cast<TextEditTabWidget*>(sender());

    QObject::connect(EditorCallProxy::instance(),
                     QOverload<const newlsp::ProjectKey &, const QString &>::of(&EditorCallProxy::toOpenFile),
                     textEditTabWidget, QOverload<const newlsp::ProjectKey &, const QString &>::of(&TextEditTabWidget::openFile));
}

void TextEditSplitter::closeSplit()
{
    if (count == 1) {
        return;
    }
    auto textEditTabWidget = qobject_cast<TextEditTabWidget*>(sender());

    tabWidgets.removeOne(textEditTabWidget);
    --count;
    textEditTabWidget->~TextEditTabWidget();
}

TextEditSplitter *TextEditSplitter::instance()
{
    if (!ins)
        ins = new TextEditSplitter;
    return ins;
}



