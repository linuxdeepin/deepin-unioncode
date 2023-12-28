// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditsplitter.h"
#include "transceiver/codeeditorreceiver.h"
#include "mainframe/texteditkeeper.h"

#include <QBoxLayout>
#include <QDebug>

namespace Private {
static TextEditSplitter *splitter = nullptr;
}

TextEditSplitter::TextEditSplitter(QWidget *parent)
    : QWidget (parent)
    , vLayout(new QVBoxLayout)
    , rootSplitter(new QSplitter)
{    
    tabWidget = new TextEditTabWidget(rootSplitter);
    rootSplitter->addWidget(tabWidget);
    rootSplitter->setHandleWidth(0);
    rootSplitter->setOpaqueResize(true);
    rootSplitter->setChildrenCollapsible(false);
    tabWidgets.insert(tabWidget, true);
    tabWidget->setCloseButtonVisible(false);
    tabWidget->setSplitButtonVisible(false);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                     tabWidget, &TextEditTabWidget::openFileWithKey);
    QObject::connect(tabWidget, &TextEditTabWidget::splitClicked,
                     this, &TextEditSplitter::doSplit);
    QObject::connect(tabWidget, &TextEditTabWidget::closed,
                     this, &TextEditSplitter::doClose);
    QObject::connect(tabWidget, &TextEditTabWidget::selected,
                     this, &TextEditSplitter::doSelected);
    QObject::connect(tabWidget, &TextEditTabWidget::closeWidget,
                     this, &TextEditSplitter::doClose);
    QObject::connect(tabWidget, &TextEditTabWidget::sigOpenFile,
                     this, &TextEditSplitter::doShowSplit);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toAddDebugPoint,
                     tabWidget, &TextEditTabWidget::addDebugPoint);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toRemoveDebugPoint,
                     tabWidget, &TextEditTabWidget::removeDebugPoint);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch(20);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(rootSplitter);
    vLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(vLayout);
}

QString TextEditSplitter::getSelectedText()
{
    auto edit = TextEditKeeper::instance()->getActiveTextEdit();
    if (!edit)
        return "";

    return edit->getSelectedText();
}

QString TextEditSplitter::getCursorBeforeText()
{
    auto edit = TextEditKeeper::instance()->getActiveTextEdit();
    if (!edit)
        return "";

    return edit->getCursorBeforeText();
}

QString TextEditSplitter::getCursorAfterText()
{
    auto edit = TextEditKeeper::instance()->getActiveTextEdit();
    if (!edit)
        return "";

    return edit->getCursorAfterText();
}

void TextEditSplitter::replaceSelectedText(const QString &text)
{
    auto edit = TextEditKeeper::instance()->getActiveTextEdit();
    if (!edit)
        return;

    edit->replaceSelectedRange(text);
}

void TextEditSplitter::showTips(const QString &tips)
{
    auto edit = TextEditKeeper::instance()->getActiveTextEdit();
    if (!edit)
        return;

    edit->showTips(tips);
}

void TextEditSplitter::insertText(const QString &text)
{
    auto edit = TextEditKeeper::instance()->getActiveTextEdit();
    if (!edit)
        return;

    edit->insertText(text);
}

void TextEditSplitter::undo()
{
    auto edit = TextEditKeeper::instance()->getActiveTextEdit();
    if (!edit)
        return;

    edit->undo();
}

TextEditSplitter::~TextEditSplitter()
{

}

void TextEditSplitter::doSplit(Qt::Orientation orientation, const newlsp::ProjectKey &key, const QString &file)
{
    auto oldEditWidget = qobject_cast<TextEditTabWidget*>(sender());
    if (!oldEditWidget)
        return;

    auto parentSplitter = getParentSplitter(oldEditWidget);
    if (parentSplitter == rootSplitter)
        rootSplit(oldEditWidget, orientation, key, file);
    else
        childSplit(oldEditWidget, orientation, key, file);
}

void TextEditSplitter::rootSplit(TextEditTabWidget *oldEditWidget, Qt::Orientation orientation,
                                               const newlsp::ProjectKey &key, const QString &file)
{
    TextEditTabWidget *newEditWidget = new TextEditTabWidget();

    if (rootSplitter->count() != 1) {
        childSplit(oldEditWidget, orientation, key, file);
        return;
    }
    rootSplitter->setOrientation(orientation);
    rootSplitter->addWidget(newEditWidget);
    if (key.isValid()) {
        newEditWidget->openFileWithKey(key, file);
    }
    splitUpdate(oldEditWidget, newEditWidget);
}

void TextEditSplitter::childSplit(TextEditTabWidget *oldEditWidget, Qt::Orientation orientation,
                                   const newlsp::ProjectKey &key, const QString &file)
{
    auto parentSplitter = getParentSplitter(oldEditWidget);
    int index = parentSplitter->indexOf(oldEditWidget);

    TextEditTabWidget *newEditWidget = new TextEditTabWidget();
    QSplitter *newSplitter = new QSplitter();
    newSplitter->setOrientation(orientation);
    newSplitter->setHandleWidth(0);
    newSplitter->setOpaqueResize(true);
    newSplitter->setChildrenCollapsible(false);
    newSplitter->addWidget(oldEditWidget);
    newSplitter->addWidget(newEditWidget);

    parentSplitter->insertWidget(index, newSplitter);

    if (key.isValid()) {
        newEditWidget->openFileWithKey(key, file);
    }
    splitUpdate(oldEditWidget, newEditWidget);
}

void TextEditSplitter::splitUpdate(TextEditTabWidget *oldEditWidget, TextEditTabWidget *newEditWidget)
{
    tabWidgets.insert(newEditWidget, true);
    tabWidgets[oldEditWidget] = false;
    oldEditWidget->setCloseButtonVisible(true);

    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                        oldEditWidget, &TextEditTabWidget::openFileWithKey);

    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toJumpFileLineWithKey,
                        oldEditWidget, &TextEditTabWidget::jumpToLineWithKey);

    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toRunFileLineWithKey,
                        oldEditWidget, &TextEditTabWidget::runningToLineWithKey);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                     newEditWidget, &TextEditTabWidget::openFileWithKey);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toAddDebugPoint,
                     newEditWidget, &TextEditTabWidget::addDebugPoint);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toRemoveDebugPoint,
                     newEditWidget, &TextEditTabWidget::removeDebugPoint);

    // connect selected texteditwidget sig-slot bind, from texteditwidget focus
    QObject::connect(newEditWidget, &TextEditTabWidget::splitClicked,
                     this, &TextEditSplitter::doSplit);

    QObject::connect(newEditWidget, &TextEditTabWidget::selected,
                     this, &TextEditSplitter::doSelected);

    QObject::connect(newEditWidget, &TextEditTabWidget::closed,
                     this, &TextEditSplitter::doClose);
    QObject::connect(newEditWidget, &TextEditTabWidget::closeWidget,
                     this, &TextEditSplitter::doClose);
    QObject::connect(newEditWidget, &TextEditTabWidget::sigOpenFile,
                     this, &TextEditSplitter::doShowSplit);
}

QSplitter *TextEditSplitter::getParentSplitter(QWidget *widget)
{
    if (!widget)
        return {};
    return qobject_cast<QSplitter *>(widget->parent());
}

bool TextEditSplitter::isEditWidget(QWidget *widget)
{
    TextEditTabWidget *editWidget = new TextEditTabWidget();
    return widget->metaObject()->className() == editWidget->metaObject()->className();
}

void TextEditSplitter::doClose()
{
    auto closedEditWidget = qobject_cast<TextEditTabWidget*>(sender());
    if (!closedEditWidget)
        return;

    auto parentSplitter = getParentSplitter(closedEditWidget);
    if (parentSplitter == rootSplitter)
        rootClose(parentSplitter, closedEditWidget);
    else
        childClose(parentSplitter, closedEditWidget);
}

void TextEditSplitter::rootClose(QSplitter *splitter, TextEditTabWidget *closedEditWidget)
{
    closeUpdate(closedEditWidget);
    delete closedEditWidget;
    tabWidgets.remove(closedEditWidget);

    if (tabWidgets.count() == 0) {
        TextEditTabWidget *newEditWidget = new TextEditTabWidget();
        newEditWidget->setCloseButtonVisible(false);
        newEditWidget->setSplitButtonVisible(false);
        splitter->addWidget(newEditWidget);
        tabWidgets.insert(newEditWidget, true);
        return;
    }

    QWidget *widget = splitter->widget(0);
    if (isEditWidget(widget)) {
        TextEditTabWidget *editWidget = qobject_cast<TextEditTabWidget*>(widget);
        editWidget->setCloseButtonVisible(false);
    }
    else
        rootSplitter = qobject_cast<QSplitter*>(widget);
}

void TextEditSplitter::childClose(QSplitter *splitter, TextEditTabWidget *closedEditWidget)
{
    closeUpdate(closedEditWidget);
    QSplitter *parentSplitter = qobject_cast<QSplitter*>(splitter->parent());
    int index = parentSplitter->indexOf(splitter);
    QWidget *anotherWidget = splitter->widget(!splitter->indexOf(closedEditWidget));

    if (isEditWidget(anotherWidget)) {
        TextEditTabWidget *editWidget = qobject_cast<TextEditTabWidget*>(anotherWidget);
        parentSplitter->insertWidget(index, editWidget);

        tabWidgets.remove(closedEditWidget);
        delete closedEditWidget;
        delete splitter;

        return;
    }
    QSplitter *replaceSplitter = qobject_cast<QSplitter*>(anotherWidget);
    parentSplitter->insertWidget(index, replaceSplitter);

    tabWidgets.remove(closedEditWidget);
    delete closedEditWidget;
    delete splitter;
}

void TextEditSplitter::closeUpdate(TextEditTabWidget *closedEditWidget)
{
    if (tabWidgets.count() == 1) {
        return;
    }
    auto it = tabWidgets.begin();
    if (it.key() == closedEditWidget)
        ++it;
    it.value() = true;
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                     it.key(), &TextEditTabWidget::openFileWithKey);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toJumpFileLineWithKey,
                     it.key(), &TextEditTabWidget::jumpToLineWithKey);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toRunFileLineWithKey,
                     it.key(), &TextEditTabWidget::runningToLineWithKey);
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSetModifiedAutoReload,
                     it.key(), &TextEditTabWidget::setModifiedAutoReload);
}

void TextEditSplitter::doSelected(bool state)
{
    auto textEditTabWidget = qobject_cast<TextEditTabWidget*>(sender());
    if (!textEditTabWidget)
        return;

    auto findIsConnEdit = [=]() -> TextEditTabWidget*
    {
        for(auto it = tabWidgets.begin(); it != tabWidgets.end(); ++it) {
            if (it.value() == true)
                return it.key();
        }
        return nullptr;
    };

    if (tabWidgets.values().contains(true)) {
        if (state) {
            while (tabWidgets.values().contains(true)) {
                auto edit = findIsConnEdit();
                if (edit) {
                    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                                        edit, &TextEditTabWidget::openFileWithKey);
                    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toJumpFileLineWithKey,
                                        edit, &TextEditTabWidget::jumpToLineWithKey);
                    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toRunFileLineWithKey,
                                        edit, &TextEditTabWidget::runningToLineWithKey);
                    QObject::disconnect(EditorCallProxy::instance(), &EditorCallProxy::toSetModifiedAutoReload,
                                        edit, &TextEditTabWidget::setModifiedAutoReload);
                    tabWidgets[edit] = false;
                }
            }
            QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toAddDebugPoint,
                             textEditTabWidget, &TextEditTabWidget::addDebugPoint);
            QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toRemoveDebugPoint,
                             textEditTabWidget, &TextEditTabWidget::removeDebugPoint);
            QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toOpenFileWithKey,
                             textEditTabWidget, &TextEditTabWidget::openFileWithKey);
            QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toJumpFileLineWithKey,
                             textEditTabWidget, &TextEditTabWidget::jumpToLineWithKey);
            QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toRunFileLineWithKey,
                             textEditTabWidget, &TextEditTabWidget::runningToLineWithKey);
            QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSetModifiedAutoReload,
                             textEditTabWidget, &TextEditTabWidget::setModifiedAutoReload);
            tabWidgets[textEditTabWidget] = state;
        }
    }
}

void TextEditSplitter::doShowSplit()
{
    if (tabWidgets.size() > 1)
        return;
    auto textEditTabWidget = qobject_cast<TextEditTabWidget*>(sender());
    textEditTabWidget->setSplitButtonVisible(true);
}

TextEditSplitter *TextEditSplitter::instance()
{
    if (!Private::splitter)
        Private::splitter = new TextEditSplitter;
    return Private::splitter;
}
