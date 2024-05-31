// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "consolewidget.h"
#include "services/project/projectservice.h"

#include <DMenu>

#include <QDebug>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class ConsoleWidgetPrivate
{
public:
    DMenu *menu = nullptr;
    QAction *consoleCopy = nullptr;
    QAction *consolePaste = nullptr;
    QAction *enterCurrentPath = nullptr;

    ProjectService *prjService = nullptr;
};

ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QTermWidget(parent),
     d(new ConsoleWidgetPrivate())
{
    setMargin(0);
    setForegroundRole(QPalette::ColorRole::Window);
    setAutoFillBackground(true);
    setTerminalOpacity(1);

    auto theme = DGuiApplicationHelper::instance()->themeType();
    updateColorScheme(theme);
    if (availableKeyBindings().contains("linux"))
        setKeyBindings("linux");

    setScrollBarPosition(QTermWidget::ScrollBarRight);
    setTerminalSizeHint(false);
    setAutoClose(false);

    d->prjService = dpfGetService(ProjectService);
    changeDir(d->prjService->getActiveProjectInfo().workspaceFolder());
    sendText("clear\n");

    d->consoleCopy = new QAction(tr("copy"), this);
    d->consolePaste = new QAction(tr("paste"), this);
    d->enterCurrentPath = new QAction(tr("Enter current project root path"), this);
    QObject::connect(d->consoleCopy, &QAction::triggered, this, &QTermWidget::copyClipboard);
    QObject::connect(d->consolePaste, &QAction::triggered, this, &QTermWidget::pasteClipboard);
    QObject::connect(d->enterCurrentPath, &QAction::triggered, this, &ConsoleWidget::enterCurrentProjectPath);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &ConsoleWidget::updateColorScheme);
}

ConsoleWidget::~ConsoleWidget()
{
    delete d;
}

void ConsoleWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (nullptr == d->menu) {
        d->menu = new DMenu(this);
        d->menu->setParent(this);
        d->menu->addAction(d->consoleCopy);
        d->menu->addAction(d->consolePaste);
        d->menu->addAction(d->enterCurrentPath);
    }
    if (selectedText().isEmpty()) {
        d->consoleCopy->setEnabled(false);
    } else {
        d->consoleCopy->setEnabled(true);
    }

    if (d->prjService->getActiveProjectInfo().isEmpty())
        d->enterCurrentPath->setEnabled(false);
    else
        d->enterCurrentPath->setEnabled(true);

    d->menu->exec(event->globalPos());
}

void ConsoleWidget::updateColorScheme(DGuiApplicationHelper::ColorType themetype)
{
    if (themetype == DGuiApplicationHelper::DarkType
        && availableColorSchemes().contains("Linux"))
        this->setColorScheme("Linux");
    else if (availableColorSchemes().contains("BlackOnWhite"))
        this->setColorScheme("BlackOnWhite");
}

void ConsoleWidget::enterCurrentProjectPath()
{
    auto path = d->prjService->getActiveProjectInfo().workspaceFolder();
    changeDir(path);
}
