// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainframe.h"
#include "generatedialog.h"
#include "services/window/windowservice.h"

#include <DPushButton>
#include <DButtonBox>
#include <DStackedWidget>
#include <DTableWidget>
#include <DLineEdit>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QAction>

#include <memory>

DWIDGET_USE_NAMESPACE
class MainFramePrivate
{
    friend class MainFrame;

    DPushButton *generateBtn { nullptr };
    GenerateDialog genDialog;
    DButtonBox *btnBox { nullptr };
    DButtonBoxButton *installedBtn { nullptr };
    DButtonBoxButton *repositoryBtn { nullptr };
    DButtonBoxButton *runningBtn { nullptr };
    DStackedWidget *stackWidget { nullptr };

    DWidget *search { nullptr };
    DLineEdit *searchEdit { nullptr };
    DPushButton *searchBtn { nullptr };

    DTableWidget *installedTabel { nullptr };
    DTableWidget *repTable { nullptr };
    DTableWidget *runningTable { nullptr };
    std::unique_ptr<QProcess> process;
};

bool MainFrame::checkToolInstalled(const QString &tool)
{
    QProcess process;
    process.start("which", QStringList() << tool);
    process.waitForFinished();
    if (process.exitCode() != 0) {
        using namespace dpfservice;
        auto windowService = dpfGetService(WindowService);
        windowService->switchContextWidget(tr("&Application Output"));
        QString message = tr("Can not find tool named: %1\n").arg(tool);
        AppOutputPane::instance()->defaultPane()->appendText(message, OutputPane::ErrorMessage);
        message = tr("Check the repository source, and install the linglong-builder, linglong-box, and linglong-bin tools.\n reference %1").arg("https://linglong.dev/guide/start/install.html");
        AppOutputPane::instance()->defaultPane()->appendText(message, OutputPane::ErrorMessage);
        return false;
    }
    return true;
}

MainFrame::MainFrame(QWidget *parent)
    : DFrame(parent),
      d(new MainFramePrivate())
{
    initUi();
    initConnection();
}

MainFrame::~MainFrame()
{
    if (d)
        delete d;
}

void MainFrame::initUi()
{
    setLineWidth(0);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    DStyle::setFrameRadius(this, 0);

    d->generateBtn = new DPushButton(tr("New"), this);
    d->generateBtn->setFixedWidth(80);

    DFrame *topWidget = new DFrame(this);
    topWidget->setLineWidth(0);
    DStyle::setFrameRadius(topWidget, 0);
    auto topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(18, 18, 18, 18);

    //init button box
    d->btnBox = new DButtonBox(this);
    d->installedBtn = new DButtonBoxButton(tr("Installed"), this);
    d->repositoryBtn = new DButtonBoxButton(tr("Repository"), this);
    d->runningBtn = new DButtonBoxButton(tr("Running"), this);

    d->btnBox->setButtonList({ d->installedBtn, d->repositoryBtn, d->runningBtn }, true);
    d->btnBox->setFixedWidth(360);
    d->installedBtn->setChecked(true);

    //init search
    d->search = new DWidget(this);
    d->searchEdit = new DLineEdit(this);
    d->searchEdit->setFixedSize(265, 36);
    d->searchBtn = new DPushButton(tr("Search"), this);
    d->searchBtn->setEnabled(false);
    auto searchLayout = new QHBoxLayout(d->search);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->addWidget(d->searchEdit);
    searchLayout->addWidget(d->searchBtn);
    d->search->hide();

    topLayout->addWidget(d->btnBox, 1, Qt::AlignLeft);
    topLayout->addWidget(d->search, 0, Qt::AlignRight);
    topLayout->addWidget(d->generateBtn, 0, Qt::AlignRight);

    d->stackWidget = new DStackedWidget(this);
    d->stackWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mainLayout->addWidget(topWidget);
    mainLayout->addWidget(d->stackWidget);

    //init table widget
    initTable();
}

void MainFrame::initTable()
{
    auto initTable = [](QTableWidget *table) {
        table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        table->verticalHeader()->hide();
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        table->horizontalHeader()->setStretchLastSection(true);
        table->horizontalHeader()->setFixedHeight(36);
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        table->setEditTriggers(QTableWidget::NoEditTriggers);
        table->setFrameShape(QFrame::NoFrame);
        table->setShowGrid(false);
        table->setContextMenuPolicy(Qt::ActionsContextMenu);
    };

    d->repTable = new QTableWidget(0, 7, this);
    initTable(d->repTable);
    d->repTable->setHorizontalHeaderLabels({ "appid", "name", "version", "arch", "channel", "module", "description" });
    d->installedTabel = new QTableWidget(0, 7, this);
    d->installedTabel->setHorizontalHeaderLabels({ "appid", "name", "version", "arch", "channel", "module", "description" });
    initTable(d->installedTabel);
    d->runningTable = new QTableWidget(0, 4, this);
    d->runningTable->setHorizontalHeaderLabels({ "App", "Pid", "Path", "ContainerID" });
    initTable(d->runningTable);

    d->stackWidget->addWidget(d->installedTabel);
    d->stackWidget->addWidget(d->repTable);
    d->stackWidget->addWidget(d->runningTable);
    d->stackWidget->setCurrentWidget(d->repTable);

    auto runAction = new QAction(tr("Run"), this);
    auto installAction = new QAction(tr("Install"), this);
    auto uninstallAction = new QAction(tr("UnInstall"), this);
    // todo : asyn / process output
    runAction->setEnabled(false);
    installAction->setEnabled(false);
    uninstallAction->setEnabled(false);
    d->repTable->addAction(installAction);
    d->installedTabel->addAction(runAction);
    d->installedTabel->addAction(uninstallAction);

    connect(runAction, &QAction::triggered, this, [=]() {
        auto selectedRow = d->installedTabel->selectionModel()->currentIndex().row();
        auto index = d->installedTabel->model()->index(selectedRow, 0);
        auto appId = d->installedTabel->model()->data(index).toString();
        runApp(appId);
    });
    connect(installAction, &QAction::triggered, this, [=]() {
        auto selectedRow = d->repTable->selectionModel()->currentIndex().row();
        auto index = d->repTable->model()->index(selectedRow, 0);
        auto appId = d->repTable->model()->data(index).toString();
        installApp(appId);
    });
    connect(uninstallAction, &QAction::triggered, this, [=]() {
        auto selectedRow = d->installedTabel->selectionModel()->currentIndex().row();
        auto index = d->installedTabel->model()->index(selectedRow, 0);
        auto appId = d->installedTabel->model()->data(index).toString();
        uninstallApp(appId);
    });

    updateInstalled();
}

void MainFrame::initConnection()
{
    connect(d->generateBtn, &QPushButton::clicked, this, [=]() {
        d->genDialog.exec();
    });
    connect(d->searchEdit, &DLineEdit::textChanged, this, [=](const QString &text) {
        d->searchBtn->setEnabled(text.isEmpty() ? false : true);
    });
    connect(d->searchBtn, &QPushButton::clicked, this, [=]() {
        updateRepository(d->searchEdit->text());
    });
    connect(d->installedBtn, &DButtonBoxButton::clicked, this, [=]() {
        d->stackWidget->setCurrentWidget(d->installedTabel);
        d->search->hide();
        updateInstalled();
    });
    connect(d->runningBtn, &DButtonBoxButton::clicked, this, [=]() {
        d->stackWidget->setCurrentWidget(d->runningTable);
        d->search->hide();
        updateRunning();
    });
    connect(d->repositoryBtn, &DButtonBoxButton::clicked, this, [=]() {
        d->stackWidget->setCurrentWidget(d->repTable);
        d->search->show();
    });
}

void MainFrame::updateInstalled()
{
    d->installedTabel->clearContents();
    d->installedTabel->setRowCount(0);

    d->process.reset(new QProcess(this));
    d->process->setProgram("ll-cli");
    QStringList args;
    args.append("list");
    args.append("--json");
    d->process->setArguments(args);

    connect(d->process.get(), &QProcess::readyRead, this, [=]() {
        auto data = d->process->readAll();

        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qCritical() << "JSON parse error: " << error.errorString();
            return;
        }

        QJsonArray arr = jsonDocument.array();
        int row = 0;
        for (auto line : arr) {
            d->installedTabel->setRowCount(row + 1);
            auto jsonObject = line.toObject();
            d->installedTabel->setItem(row, 0, new QTableWidgetItem(jsonObject["appid"].toString()));
            d->installedTabel->setItem(row, 1, new QTableWidgetItem(jsonObject["name"].toString()));
            d->installedTabel->setItem(row, 2, new QTableWidgetItem(jsonObject["version"].toString()));
            d->installedTabel->setItem(row, 3, new QTableWidgetItem(jsonObject["arch"].toString()));
            d->installedTabel->setItem(row, 4, new QTableWidgetItem(jsonObject["channel"].toString()));
            d->installedTabel->setItem(row, 5, new QTableWidgetItem(jsonObject["module"].toString()));
            d->installedTabel->setItem(row, 6, new QTableWidgetItem(jsonObject["description"].toString()));
            row++;
        }
    });

    d->process->start();
    d->process->waitForFinished();
}

void MainFrame::updateRepository(const QString &text)
{
    if (!checkToolInstalled("ll-cli"))
        return;

    d->repTable->clearContents();
    d->repTable->setRowCount(0);

    d->process.reset(new QProcess(this));
    d->process->setProgram("ll-cli");
    QStringList args;
    args.append("search");
    args.append(text);
    args.append("--json");
    d->process->setArguments(args);

    connect(d->process.get(), &QProcess::readyRead, this, [=]() {
        auto data = d->process->readAll();

        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qCritical() << "JSON parse error: " << error.errorString();
            return;
        }

        QJsonArray arr = jsonDocument.array();
        int row = 0;
        for (auto line : arr) {
            d->repTable->setRowCount(row + 1);
            auto jsonObject = line.toObject();
            d->repTable->setItem(row, 0, new QTableWidgetItem(jsonObject["appid"].toString()));
            d->repTable->setItem(row, 1, new QTableWidgetItem(jsonObject["name"].toString()));
            d->repTable->setItem(row, 2, new QTableWidgetItem(jsonObject["version"].toString()));
            d->repTable->setItem(row, 3, new QTableWidgetItem(jsonObject["arch"].toString()));
            d->repTable->setItem(row, 4, new QTableWidgetItem(jsonObject["channel"].toString()));
            d->repTable->setItem(row, 5, new QTableWidgetItem(jsonObject["module"].toString()));
            d->repTable->setItem(row, 6, new QTableWidgetItem(jsonObject["description"].toString()));
            row++;
        }
    });

    d->process->start();
    d->process->waitForFinished();
}

void MainFrame::updateRunning()
{
    d->runningTable->clearContents();
    d->runningTable->setRowCount(0);

    d->process.reset(new QProcess(this));
    d->process->setProgram("ll-cli");
    QStringList args;
    args.append("ps");
    args.append("--json");
    d->process->setArguments(args);

    connect(d->process.get(), &QProcess::readyRead, this, [=]() {
        auto data = d->process->readAll();

        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qCritical() << "JSON parse error: " << error.errorString();
            return;
        }

        QJsonArray arr = jsonDocument.array();
        int row = 0;
        for (auto line : arr) {
            d->runningTable->setRowCount(row + 1);
            auto jsonObject = line.toObject();
            d->runningTable->setItem(row, 0, new QTableWidgetItem(jsonObject["package"].toString()));
            d->runningTable->setItem(row, 1, new QTableWidgetItem(QString::number(jsonObject["pid"].toInt())));
            d->runningTable->setItem(row, 2, new QTableWidgetItem(jsonObject["path"].toString()));
            d->runningTable->setItem(row, 3, new QTableWidgetItem(jsonObject["id"].toString()));
            row++;
        }
    });

    d->process->start();
    d->process->waitForFinished();
}

void MainFrame::runApp(const QString &appId)
{
    d->process.reset(new QProcess(this));
    d->process->setProgram("ll-cli");
    QStringList args;
    args.append("run");
    args.append(appId);
    d->process->setArguments(args);

    d->process->start();
    d->process->waitForFinished();
}

void MainFrame::installApp(const QString &appId)
{
    d->process.reset(new QProcess(this));
    d->process->setProgram("ll-cli");
    QStringList args;
    args.append("install");
    args.append(appId);
    d->process->setArguments(args);

    d->process->start();
    d->process->waitForFinished();

    updateInstalled();
}

void MainFrame::uninstallApp(const QString &appId)
{
    d->process.reset(new QProcess(this));
    d->process->setProgram("ll-cli");
    QStringList args;
    args.append("uninstall");
    args.append(appId);
    d->process->setArguments(args);

    d->process->start();
    d->process->waitForFinished();

    updateInstalled();
}

void MainFrame::output(const QString &text, OutputPane::OutputFormat format)
{
    AppOutputPane::instance()->defaultPane()->appendText(text, format);
}
