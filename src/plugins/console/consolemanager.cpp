// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "consolemanager.h"

#include <DToolButton>
#include <DStackedWidget>
#include <DStyle>
#include <DDialog>
#include <DFrame>

#include <QUuid>
#include <QProcess>
#include <QHBoxLayout>
#include <QListView>
#include <QSplitter>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE

const int IdRole = Qt::UserRole + 1;

class ConsoleManagerPrivate : public QObject
{
public:
    explicit ConsoleManagerPrivate(ConsoleManager *qq);

    void initUI();
    void initConnection();

    void createDefaultConsole();
    void appendConsole();
    void removeConsole();
    void switchConsole(const QModelIndex &index, const QModelIndex &previous);
    void switchConsole(const QUuid &uuid);
    void updateButtonState();

public:
    ConsoleManager *q;

    QMap<QString, ConsoleWidget *> consoleMap;
    DStackedWidget *consoleStackedWidget { nullptr };
    QListView *consoleListView { nullptr };
    QStandardItemModel *model { nullptr };
    DToolButton *addConsoleBtn { nullptr };
    DToolButton *removeConsoleBtn { nullptr };
};

ConsoleManagerPrivate::ConsoleManagerPrivate(ConsoleManager *qq)
    : q(qq)
{
}

void ConsoleManagerPrivate::initUI()
{
    auto mainLayout = new QHBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    consoleStackedWidget = new DStackedWidget(q);
    consoleStackedWidget->setMinimumWidth(500);
    consoleStackedWidget->setContentsMargins(0, 0, 0, 0);

    DFrame *listViewFrame = new DFrame(q);
    listViewFrame->setLineWidth(0);
    DStyle::setFrameRadius(listViewFrame, 0);

    consoleListView = new QListView(listViewFrame);
    consoleListView->setLineWidth(0);
    model = new QStandardItemModel(consoleListView);
    consoleListView->setModel(model);

    addConsoleBtn = new DToolButton(listViewFrame);
    removeConsoleBtn = new DToolButton(listViewFrame);

    addConsoleBtn->setIcon(QIcon::fromTheme("binarytools_add"));
    removeConsoleBtn->setIcon(QIcon::fromTheme("binarytools_reduce"));
    removeConsoleBtn->setEnabled(false);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(addConsoleBtn);
    btnLayout->addWidget(removeConsoleBtn);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setAlignment(Qt::AlignLeft);
    btnLayout->setSpacing(5);

    QVBoxLayout *listViewLayout = new QVBoxLayout(listViewFrame);
    listViewLayout->addLayout(btnLayout);
    listViewLayout->addWidget(consoleListView);
    listViewLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(q);
    splitter->addWidget(consoleStackedWidget);
    splitter->addWidget(listViewFrame);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setHandleWidth(2);

    mainLayout->addWidget(splitter);
}

void ConsoleManagerPrivate::initConnection()
{
    connect(consoleListView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, QOverload<const QModelIndex &, const QModelIndex &>::of(&ConsoleManagerPrivate::switchConsole));
    connect(addConsoleBtn, &DToolButton::clicked, this, &ConsoleManagerPrivate::appendConsole);
    connect(removeConsoleBtn, &DToolButton::clicked, this, &ConsoleManagerPrivate::removeConsole);
    connect(model, &QStandardItemModel::rowsRemoved, this, &ConsoleManagerPrivate::updateButtonState);
    connect(model, &QStandardItemModel::rowsInserted, this, &ConsoleManagerPrivate::updateButtonState);
}

void ConsoleManagerPrivate::createDefaultConsole()
{
    q->createConsole(ConsoleManager::tr("Terminal"));
}

void ConsoleManagerPrivate::appendConsole()
{
    q->createConsole(ConsoleManager::tr("New Terminal"));
}

void ConsoleManagerPrivate::removeConsole()
{
    auto index = consoleListView->currentIndex();
    if (!index.isValid())
        return;

    auto id = index.data(IdRole).toString();
    if (auto console = q->findConsole(id)) {
        consoleStackedWidget->removeWidget(console);
        console->deleteLater();
    }

    model->removeRow(index.row());
    consoleMap.remove(id);
}

void ConsoleManagerPrivate::switchConsole(const QModelIndex &index, const QModelIndex &previous)
{
    auto id = index.data(IdRole).toString();
    if (auto console = q->findConsole(id)) {
        console->setFocus();
        consoleStackedWidget->setCurrentWidget(console);
    }
}

void ConsoleManagerPrivate::switchConsole(const QUuid &uuid) {
    if (auto console = q->findConsole(uuid.toString())) {
        console->setFocus();
        consoleStackedWidget->setCurrentWidget(console);
    }
}

void ConsoleManagerPrivate::updateButtonState()
{
    removeConsoleBtn->setEnabled(model->rowCount() > 1);
}

ConsoleManager::ConsoleManager(QWidget *parent)
    : QWidget(parent),
      d(new ConsoleManagerPrivate(this))
{
    d->initUI();
    d->initConnection();
}

ConsoleManager::~ConsoleManager()
{
    delete d;
}

QTermWidget *ConsoleManager::currentConsole()
{
    //todo: when switch to other project`s path, open a new console
    auto index = d->consoleListView->currentIndex();
    if (!index.isValid())
        return nullptr;

    auto id = index.data(IdRole).toString();
    return findConsole(id);
}

QTermWidget *ConsoleManager::findConsole(const QString &id)
{
    return d->consoleMap.value(id, nullptr);
}

QTermWidget *ConsoleManager::createConsole(const QString &name, bool startNow, bool rename)
{
    auto id = QUuid::createUuid().toString();
    ConsoleWidget *console = new ConsoleWidget(this, startNow);
    d->consoleMap.insert(id, console);
    d->consoleStackedWidget->addWidget(console);

    QStandardItem *item = new QStandardItem(name);
    item->setData(id, IdRole);
    item->setEditable(rename);
    d->model->appendRow(item);
    d->consoleListView->setCurrentIndex(d->model->index(d->model->rowCount() - 1, 0));

    return console;
}

void ConsoleManager::sendCommand(const QString &text)
{
    if (!currentConsole())
        d->createDefaultConsole();

    currentConsole()->sendText(text);
}

void ConsoleManager::executeCommand(const QString &name, const QString &program, const QStringList &args, const QString &workingDir, const QStringList &env)
{
    auto startNow = env.isEmpty(); // console start after set environment
    auto console = createConsole(name, startNow);
    if (!env.isEmpty()) {
        console->setEnvironment(env);
        console->startShellProgram();
    }

    auto dir = workingDir;
    if (!dir.isEmpty() && QFile::exists(dir)) {
        console->changeDir(dir);
        console->sendText("clear\n");
    }

    QString cmd = program + ' ';
    cmd += args.join(' ') + '\n';
    console->sendText(cmd);
}

QUuid ConsoleManager::newConsole(const QString &name, bool rename)
{
    auto ptr_term = dynamic_cast<ConsoleWidget*>(this->createConsole(name, true, rename));
    return QUuid::fromString(d->consoleMap.key(ptr_term, ""));
}

void ConsoleManager::selectConsole(const QUuid &uuid)
{
    d->switchConsole(uuid);
}

void ConsoleManager::run2Console(const QUuid &uuid, const QProcess &process)
{
    auto ptr_term = findConsole(uuid.toString());
    if (!ptr_term)
        return;

    if (!process.workingDirectory().isEmpty())
        ptr_term->changeDir(process.workingDirectory());
    if (!process.environment().isEmpty())
        ptr_term->setEnvironment(process.environment());
    if (!process.program().isEmpty()) {
        QString cmd = process.program();
        if (!process.arguments().isEmpty())
            cmd += ' ' + process.arguments().join(' ');
        cmd += '\n';
        ptr_term->sendText(cmd);
    }
}

void ConsoleManager::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (!currentConsole())
        d->createDefaultConsole();
}
