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

TextEditSplitter::TextEditSplitter(QWidget *parent)
    : QWidget (parent)
    , vLayout(new QVBoxLayout)
    , mainSplitter(new QSplitter)
{
    tabWidget = new TextEditTabWidget(mainSplitter);
    mainSplitter->addWidget(tabWidget);
    tabWidgets.append(tabWidget);
    splitters.append(mainSplitter);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                     tabWidget, &TextEditTabWidget::openFileWithKey);
    QObject::connect(tabWidget, &TextEditTabWidget::splitClicked,
                     this, &TextEditSplitter::doSplit);
    QObject::connect(tabWidget, &TextEditTabWidget::closed,
                     this, &TextEditSplitter::doClose);
    QObject::connect(tabWidget, &TextEditTabWidget::selected,
                     this, &TextEditSplitter::doSelected);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch(20);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(mainSplitter);
    this->setLayout(vLayout);

}

TextEditSplitter::~TextEditSplitter()
{

}

void TextEditSplitter::doSplit(Qt::Orientation orientation, const newlsp::ProjectKey &key, const QString &file)
{
    auto oldEditWidget = qobject_cast<TextEditTabWidget*>(sender());
    if (!oldEditWidget)
        return;
    auto splitter = qobject_cast<QSplitter *>(oldEditWidget->parent());
    QSplitter *newSplitter = new QSplitter(splitter);
    splitters.append(newSplitter);
    newSplitter->setOrientation(orientation);
    newSplitter->setHandleWidth(0);
    oldEditWidget->setParent(newSplitter);

    TextEditTabWidget *newEditWidget = new TextEditTabWidget(newSplitter);
    tabWidgets.append(newEditWidget);
    if (key.isValid()) {
        newEditWidget->openFileWithKey(key, file);
    }
    newEditWidget->setFocus();
    newEditWidget->activateWindow();
    newSplitter->insertWidget(0, oldEditWidget);
    newSplitter->insertWidget(1, newEditWidget);
    splitter->replaceWidget(0, newSplitter);

    // cancel all open file sig-slot from texteditwidget
    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                        oldEditWidget, &TextEditTabWidget::openFileWithKey);

    // connect new texteditwidget openfile slot
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                     newEditWidget, &TextEditTabWidget::openFileWithKey);

    // connect selected texteditwidget sig-slot bind, from texteditwidget focus
    QObject::connect(newEditWidget, &TextEditTabWidget::splitClicked,
                     this, &TextEditSplitter::doSplit);

    QObject::connect(newEditWidget, &TextEditTabWidget::selected,
                     this, &TextEditSplitter::doSelected);

    QObject::connect(newEditWidget, &TextEditTabWidget::closed,
                     this, &TextEditSplitter::doClose);
}

void TextEditSplitter::doSelected(bool state)
{
    auto textEditTabWidget = qobject_cast<TextEditTabWidget*>(sender());
    if (!textEditTabWidget)
        return;

    if (state) {
        QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                         textEditTabWidget, &TextEditTabWidget::openFileWithKey);
    } else {
        QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                            textEditTabWidget, &TextEditTabWidget::openFileWithKey);
        bool isConnect = false;
        for (int i = 0; i < tabWidgets.size(); i++) {
            isConnect = connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                                     tabWidgets.at(i), &TextEditTabWidget::openFileWithKey);
            if (isConnect) {
                break;
            }
        }
        if (!isConnect) {
            QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                                textEditTabWidget, &TextEditTabWidget::openFileWithKey);
        }
    }
}

TextEditSplitter *TextEditSplitter::instance()
{
    static TextEditSplitter ins;
    return &ins;
}

void TextEditSplitter::doClose()
{
    if(tabWidgets.size() <= 1) {
        return;
    }
    auto textEditTabWidget = qobject_cast<TextEditTabWidget*>(sender());
    int idx = tabWidgets.indexOf(textEditTabWidget);
    if (0 < idx) {
        delete tabWidgets.at(idx);
        tabWidgets.removeAt(idx);
//        delete splitters.at(idx);
//        splitters.removeAt(idx);
    }
}



