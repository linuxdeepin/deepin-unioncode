// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "consolemanager.h"

#include <DToolButton>
#include <DStackedWidget>
#include <DStyle>
#include <DDialog>
#include <DFrame>

#include <QHBoxLayout>
#include <QListView>
#include <QSplitter>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE

class ConsoleManagerPrivate
{
public:
    friend class ConsoleManager;

    QHBoxLayout *mainLayout { nullptr };
    ConsoleWidget *currentConsole { nullptr };
    QList<ConsoleWidget *> consoleList;
    DStackedWidget *consoleStackedWidget { nullptr };
    QListView *consoleListView { nullptr };
    QStandardItemModel *model { nullptr };
    DToolButton *addConsoleBtn { nullptr };
    DToolButton *removeConsoleBtn { nullptr };
};

ConsoleManager::ConsoleManager(QWidget *parent)
    : QWidget(parent), d(new ConsoleManagerPrivate)
{
    initUI();
    initConnection();
}

ConsoleManager::~ConsoleManager()
{
    delete d;
}

void ConsoleManager::initUI()
{
    d->mainLayout = new QHBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);

    d->consoleStackedWidget = new DStackedWidget(this);
    d->consoleStackedWidget->setMinimumWidth(500);
    d->consoleStackedWidget->setContentsMargins(0, 0, 0, 0);

    DFrame *listViewFrame = new DFrame(this);
    listViewFrame->setLineWidth(0);
    DStyle::setFrameRadius(listViewFrame, 0);

    d->consoleListView = new QListView(listViewFrame);
    d->consoleListView->setLineWidth(0);
    d->model = new QStandardItemModel(d->consoleListView);
    d->consoleListView->setModel(d->model);

    d->addConsoleBtn = new DToolButton(listViewFrame);
    d->removeConsoleBtn = new DToolButton(listViewFrame);

    d->addConsoleBtn->setIcon(QIcon::fromTheme("binarytools_add"));
    d->removeConsoleBtn->setIcon(QIcon::fromTheme("binarytools_reduce"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(d->addConsoleBtn);
    btnLayout->addWidget(d->removeConsoleBtn);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setAlignment(Qt::AlignLeft);
    btnLayout->setSpacing(5);

    QVBoxLayout *listViewLayout = new QVBoxLayout(listViewFrame);
    listViewLayout->addLayout(btnLayout);
    listViewLayout->addWidget(d->consoleListView);
    listViewLayout->setContentsMargins(0, 0, 0, 0);

    d->mainLayout->addWidget(d->consoleStackedWidget);

    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(d->consoleStackedWidget);
    splitter->addWidget(listViewFrame);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setHandleWidth(2);

    d->mainLayout->addWidget(splitter);
}

void ConsoleManager::initConnection()
{
    connect(d->consoleListView, &QListView::clicked, this, &ConsoleManager::switchConsole);
    connect(d->addConsoleBtn, &DToolButton::clicked, this, &ConsoleManager::appendConsole);
    connect(d->removeConsoleBtn, &DToolButton::clicked, this, &ConsoleManager::removeConsole);
}

QTermWidget *ConsoleManager::getCurrentConsole()
{
    //todo: when switch to other project`s path, open a new console
    return d->currentConsole;
}

void ConsoleManager::executeCommand(const QString &text)
{
    if (!d->currentConsole)
        initDefaultConsole();
    d->currentConsole->sendText(text);
}

void ConsoleManager::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (!d->currentConsole)
        initDefaultConsole();
}

void ConsoleManager::initDefaultConsole()
{
    auto defaultConsole = new ConsoleWidget(this);
    d->consoleList.append(defaultConsole);
    d->currentConsole = defaultConsole;
    d->consoleStackedWidget->addWidget(defaultConsole);

    QStandardItem *item = new QStandardItem(tr("Console"));
    d->model->appendRow(item);
}

void ConsoleManager::appendConsole()
{
    ConsoleWidget *console = new ConsoleWidget(this);
    d->consoleList.append(console);
    d->consoleStackedWidget->addWidget(console);
    d->consoleStackedWidget->setCurrentWidget(console);
    d->currentConsole = console;

    QStandardItem *item = new QStandardItem(tr("New Console"));
    d->model->appendRow(item);
    d->consoleListView->setCurrentIndex(d->model->index(d->model->rowCount() - 1, 0));
}

void ConsoleManager::removeConsole()
{
    if (d->consoleListView->currentIndex().row() == -1) {
        showDialog(tr("No console selected."));
        return;
    }
    if (d->consoleList.count() <= 1) {
        showDialog(tr("Cannot remove the last console."));
        return;
    }
    d->consoleStackedWidget->removeWidget(d->currentConsole);
    d->consoleList.removeOne(d->currentConsole);
    d->model->removeRow(d->consoleListView->currentIndex().row());
    delete d->currentConsole;

    d->currentConsole = d->consoleList.at(d->consoleListView->currentIndex().row());
}

void ConsoleManager::showDialog(const QString &msg)
{
    DDialog dialog;
    dialog.setIcon(QIcon::fromTheme("dialog-warning"));
    dialog.setWindowTitle(tr("Warning"));
    dialog.setMessage(msg);
    dialog.addButton(tr("OK"), true, DDialog::ButtonNormal);
    dialog.exec();
}

void ConsoleManager::switchConsole(const QModelIndex &index)
{
    d->consoleStackedWidget->setCurrentIndex(index.row());
    d->currentConsole = d->consoleList.at(index.row());
}
