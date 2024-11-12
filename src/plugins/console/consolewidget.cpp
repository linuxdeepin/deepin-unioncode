// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "consolewidget.h"
#include "generateinput.h"
#include "services/project/projectservice.h"

#include <DMenu>

#include <QDir>
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
    QAction *showInputWidget = nullptr;
    GenerateInput *inputWidget = nullptr;

    ProjectService *prjService = nullptr;
};

ConsoleWidget::ConsoleWidget(QWidget *parent, bool startNow)
    : QTermWidget(startNow, parent),
     d(new ConsoleWidgetPrivate())
{
    setMargin(0);
    setForegroundRole(QPalette::ColorRole::Window);
    setAutoFillBackground(true);
    setTerminalOpacity(1);

    // using default deepin-terminal style schemes
    // U can see schemes path with deepin-os
    QString sys_schemes_path = "/usr/share/terminalwidget5/color-schemes";
    if (QDir(sys_schemes_path).exists())
        addCustomColorSchemeDir(sys_schemes_path);

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

    d->inputWidget = new GenerateInput(this);
    d->inputWidget->hide();
    auto layout = this->layout();
    if (layout)
        layout->addWidget(d->inputWidget);

    d->consoleCopy = new QAction(tr("copy"), this);
    d->consolePaste = new QAction(tr("paste"), this);
    d->enterCurrentPath = new QAction(tr("Enter current project root path"), this);
    d->showInputWidget = new QAction(tr("Intelligent Command Generation"), this);
    QObject::connect(d->consoleCopy, &QAction::triggered, this, &QTermWidget::copyClipboard);
    QObject::connect(d->consolePaste, &QAction::triggered, this, &QTermWidget::pasteClipboard);
    QObject::connect(d->enterCurrentPath, &QAction::triggered, this, &ConsoleWidget::enterCurrentProjectPath);
    QObject::connect(d->showInputWidget, &QAction::triggered, d->inputWidget, &GenerateInput::show);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &ConsoleWidget::updateColorScheme);
    QObject::connect(d->inputWidget, &GenerateInput::commandGenerated, this, &ConsoleWidget::sendText);
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
        d->menu->addAction(d->showInputWidget);
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
    if (DGuiApplicationHelper::DarkType == themetype) {
        // need add member option setting check logic
        if (availableColorSchemes().contains("Dark"))
            this->setColorScheme("Dark");
        else
            this->setColorScheme(":/color-schemes/Dark.colorscheme");
    } else if (DGuiApplicationHelper::LightType == themetype) {
        // need add member option setting check logic
        if (availableColorSchemes().contains("Light"))
            this->setColorScheme("Light");
        else
            this->setColorScheme(":/color-schemes/Light.colorscheme");
    }
}

void ConsoleWidget::enterCurrentProjectPath()
{
    auto path = d->prjService->getActiveProjectInfo().workspaceFolder();
    changeDir(path);
}
