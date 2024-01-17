// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"
#include "private/workspacewidget_p.h"
#include "transceiver/codeeditorreceiver.h"

#include <QVBoxLayout>
#include <QVariant>

WorkspaceWidgetPrivate::WorkspaceWidgetPrivate(WorkspaceWidget *qq)
    : QObject(qq),
      q(qq)
{
}

void WorkspaceWidgetPrivate::initUI()
{
    QSplitter *splitter = new QSplitter(q);
    splitter->setHandleWidth(0);
    splitter->setOpaqueResize(true);
    splitter->setChildrenCollapsible(false);

    TabWidget *tabWidget = new TabWidget(splitter);
    tabWidget->setCloseButtonVisible(false);
    tabWidget->setSplitButtonVisible(false);
    tabWidgetList.append(tabWidget);

    connectTabWidgetSignals(tabWidget);
    splitter->addWidget(tabWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(splitter);
}

void WorkspaceWidgetPrivate::initConnection()
{
    connect(EditorCallProxy::instance(), &EditorCallProxy::openFileRequested, this, &WorkspaceWidgetPrivate::onOpenFileRequested);
    connect(qApp, &QApplication::focusChanged, this, &WorkspaceWidgetPrivate::onFocusChanged);
}

void WorkspaceWidgetPrivate::connectTabWidgetSignals(TabWidget *tabWidget)
{
    connect(tabWidget, &TabWidget::splitRequested, this, &WorkspaceWidgetPrivate::onSplitRequested);
    connect(tabWidget, &TabWidget::closeRequested, this, &WorkspaceWidgetPrivate::onCloseRequested);
}

TabWidget *WorkspaceWidgetPrivate::currentTabWidget() const
{
    if (focusTabWidget)
        return focusTabWidget;

    // Return a default tabWidget
    return tabWidgetList.isEmpty() ? nullptr : tabWidgetList.first();
}

void WorkspaceWidgetPrivate::doSplit(QSplitter *spliter, int index, const QString &fileName, int pos, int scroll)
{
    TabWidget *tabWidget = new TabWidget(spliter);
    connectTabWidgetSignals(tabWidget);

    tabWidgetList.append(tabWidget);
    spliter->insertWidget(index, tabWidget);

    tabWidget->openFile(fileName);
    // Set the cursor and scroll position
    tabWidget->setEditorCursorPosition(pos);
    tabWidget->setEditorScrollValue(scroll);
}

void WorkspaceWidgetPrivate::onSplitRequested(Qt::Orientation ori, const QString &fileName)
{
    auto tabWidgetSender = qobject_cast<TabWidget *>(sender());
    if (!tabWidgetSender)
        return;

    auto spliter = qobject_cast<QSplitter *>(tabWidgetSender->parent());
    if (!spliter)
        return;

    tabWidgetSender->setCloseButtonVisible(true);

    int index = spliter->indexOf(tabWidgetSender);
    int cursorPos = tabWidgetSender->editorCursorPosition();
    int scroll = tabWidgetSender->editorScrollValue();

    if (spliter->count() == 1) {
        // Only one widget is added to the splitter,
        // change its orientation and add a new widget
        spliter->setOrientation(ori);
        doSplit(spliter, index + 1, fileName, cursorPos, scroll);
    } else if (spliter->orientation() == ori) {
        doSplit(spliter, index + 1, fileName, cursorPos, scroll);
    } else {
        // Use a new splitter to replace
        QSplitter *newSplitter = new QSplitter(q);
        newSplitter->setOrientation(ori);

        spliter->replaceWidget(index, newSplitter);
        newSplitter->addWidget(tabWidgetSender);
        doSplit(newSplitter, 1, fileName, cursorPos, scroll);
    }
}

void WorkspaceWidgetPrivate::onCloseRequested()
{
    if (tabWidgetList.size() == 1)
        return;

    TabWidget *tabWidget = qobject_cast<TabWidget *>(sender());
    if (!tabWidget)
        return;

    if (focusTabWidget == tabWidget)
        focusTabWidget = nullptr;

    tabWidgetList.removeOne(tabWidget);
    tabWidget->deleteLater();

    if (tabWidgetList.size() == 1)
        tabWidgetList.first()->setCloseButtonVisible(false);
}

void WorkspaceWidgetPrivate::onOpenFileRequested(const QString &fileName)
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->openFile(fileName);
}

void WorkspaceWidgetPrivate::onFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)

    if (!now)
        return;

    // the `now` is TextEditor
    auto tabWidget = qobject_cast<TabWidget *>(now->parentWidget());
    if (!tabWidget)
        return;

    focusTabWidget = tabWidget;
}

WorkspaceWidget::WorkspaceWidget(QWidget *parent)
    : QWidget(parent),
      d(new WorkspaceWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

QString WorkspaceWidget::selectedText() const
{
    auto tabWidget = d->currentTabWidget();
    if (!tabWidget)
        return "";

    return tabWidget->selectedText();
}

QString WorkspaceWidget::cursorBeforeText() const
{
    auto tabWidget = d->currentTabWidget();
    if (!tabWidget)
        return "";

    return tabWidget->cursorBeforeText();
}

QString WorkspaceWidget::cursorBehindText() const
{
    auto tabWidget = d->currentTabWidget();
    if (!tabWidget)
        return "";

    return tabWidget->cursorBehindText();
}

void WorkspaceWidget::replaceSelectedText(const QString &text)
{
    auto tabWidget = d->currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->replaceSelectedText(text);
}
