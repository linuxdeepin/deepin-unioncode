// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytoolsmanager.h"
#include "mainframe/environmentview.h"
#include "mainframe/binarytoolsdialog.h"

#include "common/util/custompaths.h"
#include "common/util/eventdefinitions.h"
#include "services/window/windowservice.h"
#include "base/abstractaction.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QUuid>
#include <QTextBlock>

constexpr char GroupObject[] { "groups" };
constexpr char ToolObject[] { "tools" };
constexpr char NameKey[] { "name" };
constexpr char IdKey[] { "id" };
constexpr char DescriptionKey[] { "description" };
constexpr char TypeKey[] { "type" };
constexpr char CommandKey[] { "command" };
constexpr char ArgumentsKey[] { "arguments" };
constexpr char WorkingDirectoryKey[] { "workingDirectory" };
constexpr char OutputOptionKey[] { "outputOption" };
constexpr char ErrorOutputOptionKey[] { "errorOutputOption" };
constexpr char AddToToolbarKey[] { "addToToolbar" };
constexpr char IconKey[] { "icon" };
constexpr char EnvironmentKey[] { "environment" };

using namespace dpfservice;

QString ToolProcess::readAllStandardOutput()
{
    return std::move(stdOut);
}

QString ToolProcess::readAllStandardError()
{
    return std::move(stdError);
}

void ToolProcess::start(const QString &id)
{
    if (this->id != id)
        return;

    stdOut.clear();
    stdError.clear();

    connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, std::bind(&ToolProcess::finished, this, id, std::placeholders::_1, std::placeholders::_2));
    connect(&process, &QProcess::readyReadStandardOutput, this, [=] {
        stdOut += process.readAllStandardOutput();
        Q_EMIT readyReadStandardOutput(id);
    });
    connect(&process, &QProcess::readyReadStandardError, this, [=] {
        stdError += process.readAllStandardError();
        Q_EMIT readyReadStandardError(id);
    });

    process.setProgram(program);
    process.setArguments(arguments);
    process.setWorkingDirectory(workingDir);
    process.setProcessEnvironment(environment);

    process.start();
    process.waitForFinished(-1);
}

void ToolProcess::stop()
{
    if (process.state() != QProcess::NotRunning)
        process.kill();
}

BinaryToolsManager::BinaryToolsManager(QObject *parent)
    : QObject(parent)
{
}

BinaryToolsManager::~BinaryToolsManager()
{
    auto iter = toolTaskMap.begin();
    for (; iter != toolTaskMap.end(); ++iter) {
        stopTool(iter.key());
    }
}

QSharedPointer<ToolProcess> BinaryToolsManager::createToolProcess(const ToolInfo &tool)
{
    using namespace std::placeholders;

    QSharedPointer<ToolProcess> toolProcess { new ToolProcess };
    connect(toolProcess.data(), &ToolProcess::finished, this, &BinaryToolsManager::executeFinished, Qt::QueuedConnection);
    if (tool.outputOption == ShowInApplicationOutput)
        connect(toolProcess.data(), &ToolProcess::readyReadStandardOutput, this, &BinaryToolsManager::handleReadOutput, Qt::QueuedConnection);
    if (tool.errorOutputOption == ShowInApplicationOutput)
        connect(toolProcess.data(), &ToolProcess::readyReadStandardError, this, &BinaryToolsManager::handleReadError, Qt::QueuedConnection);
    connect(this, &BinaryToolsManager::execute, toolProcess.data(), &ToolProcess::start, Qt::QueuedConnection);

    QSharedPointer<QThread> toolThread { new QThread };
    toolProcess->moveToThread(toolThread.data());
    toolThread->start();
    toolTaskMap.insert(tool.id, std::make_tuple(toolProcess, toolThread));

    return toolProcess;
}

BinaryToolsManager *BinaryToolsManager::instance()
{
    static BinaryToolsManager ins;
    return &ins;
}

void BinaryToolsManager::save()
{
    QJsonArray groupsArray;

    auto iter = allTools.begin();
    for (; iter != allTools.end(); ++iter) {
        QJsonObject groupObject;
        groupObject[NameKey] = iter.key();

        QJsonArray itemsArray;
        for (const auto &item : iter.value()) {
            QJsonObject itemObject;
            itemObject[IdKey] = item.id;
            itemObject[NameKey] = item.name;
            itemObject[DescriptionKey] = item.description;
            itemObject[TypeKey] = item.type;
            itemObject[CommandKey] = item.command;
            itemObject[ArgumentsKey] = item.arguments;
            itemObject[WorkingDirectoryKey] = item.workingDirectory;
            itemObject[OutputOptionKey] = item.outputOption;
            itemObject[ErrorOutputOptionKey] = item.errorOutputOption;
            itemObject[AddToToolbarKey] = item.addToToolbar;
            itemObject[IconKey] = item.icon;
            itemObject[EnvironmentKey] = QJsonDocument::fromVariant(item.environment).object();

            itemsArray.append(itemObject);
        }

        groupObject[ToolObject] = itemsArray;
        groupsArray.append(groupObject);
    }

    QJsonObject groupsObject;
    groupsObject[GroupObject] = groupsArray;

    QJsonDocument doc(groupsObject);
    QString confPath = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("binarytools.json");
    QFile file(confPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void BinaryToolsManager::setTools(const QMap<QString, QList<ToolInfo>> &dataMap)
{
    allTools = dataMap;
}

ToolInfo BinaryToolsManager::findTool(const QString &id)
{
    auto iter = allTools.begin();
    for (; iter != allTools.end(); ++iter) {
        auto &list = iter.value();
        auto result = std::find_if(list.begin(), list.end(), [&id](const ToolInfo &tool) {
            return tool.id == id;
        });

        if (result != list.end())
            return ToolInfo(*result);
    }

    return {};
}

QMap<QString, QList<ToolInfo>> BinaryToolsManager::tools()
{
    if (!allTools.isEmpty())
        return allTools;

    QString confPath = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("binarytools.json");
    if (!QFile::exists(confPath))
        confPath = ":/configure/default_binarytools.json";

    QFile file(confPath);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << error.errorString();
        return {};
    }

    if (!doc.isObject())
        return {};

    QJsonObject jsonObj = doc.object();
    QJsonArray groups = jsonObj[GroupObject].toArray();

    for (const QJsonValue &groupVal : groups) {
        QJsonObject groupObj = groupVal.toObject();
        QString groupName = groupObj[NameKey].toString();
        if (groupName.isEmpty())
            groupName = tr("Default Group");

        QJsonArray items = groupObj[ToolObject].toArray();
        QList<ToolInfo> itemList;
        for (const QJsonValue &itemVal : items) {
            QJsonObject itemObj = itemVal.toObject();
            ToolInfo itemInfo;

            itemInfo.id = itemObj[IdKey].toString();
            if (itemInfo.id.isEmpty())
                itemInfo.id = QUuid::createUuid().toString(QUuid::WithoutBraces);

            itemInfo.displyGroup = groupName;
            itemInfo.name = itemObj[NameKey].toString();
            itemInfo.description = itemObj[DescriptionKey].toString();
            itemInfo.type = itemObj[TypeKey].toInt(0);
            itemInfo.command = itemObj[CommandKey].toString();
            itemInfo.arguments = itemObj[ArgumentsKey].toString();
            itemInfo.workingDirectory = itemObj[WorkingDirectoryKey].toString();
            itemInfo.outputOption = itemObj[OutputOptionKey].toInt(0);
            itemInfo.errorOutputOption = itemObj[ErrorOutputOptionKey].toInt(0);
            itemInfo.addToToolbar = itemObj[AddToToolbarKey].toBool(false);
            itemInfo.icon = itemObj[IconKey].toString();
            itemInfo.environment = itemObj[EnvironmentKey].toObject().toVariantMap();
            if (itemInfo.environment.isEmpty())
                itemInfo.environment = EnvironmentView::defaultEnvironment();

            itemList.append(itemInfo);
        }

        allTools.insert(groupName, itemList);
    }

    return allTools;
}

void BinaryToolsManager::executeTool(const QString &id)
{
    const auto &tool = this->findTool(id);
    if (!tool.isValid())
        return;

    if (!checkCommandExists(tool.command)) {
        QString msg = tr("The tool (%1) execution program does not exist. Install and run it again").arg(tool.name);
        windowSrv->notify(2, "", msg, QStringList() << "ok_default" << tr("Ok"));
        return;
    }

    auto toolProcess = createToolProcess(tool);
    QStringList argList = tool.arguments.split(" ", QString::SkipEmptyParts);
    toolProcess->setId(id);
    toolProcess->setProgram(tool.command);
    toolProcess->setArguments(argList);
    toolProcess->setWorkingDirectory(tool.workingDirectory);
    QProcessEnvironment env;
    auto iterator = tool.environment.begin();
    while (iterator != tool.environment.end()) {
        env.insert(iterator.key(), iterator.value().toString());
        ++iterator;
    }
    toolProcess->setProcessEnvironment(env);

    AppOutputPane::instance()->createApplicationPane(id, tool.name);
    auto stopHandler = std::bind(&BinaryToolsManager::stopTool, this, id);
    AppOutputPane::instance()->setStopHandler(id, stopHandler);
    QString startMsg = tr("Start execute \"%1\": \"%2\" \"%3\" in workspace \"%4\".\n")
                               .arg(tool.name, tool.command, tool.arguments, tool.workingDirectory);
    printOutput(id, startMsg, OutputPane::NormalMessage);

    Q_EMIT execute(id);
}

void BinaryToolsManager::checkAndAddToToolbar(const QMap<QString, QList<ToolInfo>> &tools)
{
    auto iter = tools.begin();
    for (; iter != tools.end(); ++iter) {
        for (const auto &tool : iter.value())
            addToToolBar(tool);
    }
}

void BinaryToolsManager::updateToolMenu(const QMap<QString, QList<ToolInfo>> &tools)
{
    if (!toolMenu)
        return;

    toolMenu->clear();
    auto iter = tools.begin();
    for (; iter != tools.end(); ++iter) {
        auto groupAct = toolMenu->addAction(iter.key());
        auto subMenu = new QMenu(toolMenu);
        groupAct->setMenu(subMenu);
        for (const auto &tool : iter.value()) {
            auto act = subMenu->addAction(QIcon::fromTheme(tool.icon), tool.name);
            connect(act, &QAction::triggered, this, std::bind(&BinaryToolsManager::executeTool, this, tool.id));
        }
    }

    toolMenu->addSeparator();
    auto configureAct = toolMenu->addAction(tr("Configure..."));
    connect(configureAct, &QAction::triggered, this, [=]() {
        BinaryToolsDialog dlg;
        dlg.exec();
    });
}

void BinaryToolsManager::setToolMenu(QMenu *menu)
{
    toolMenu = menu;
}

void BinaryToolsManager::executeFinished(const QString &id, int exitCode, QProcess::ExitStatus exitStatus)
{
    const auto &tool = findTool(id);
    if (!tool.isValid())
        return;

    QString retMsg;
    if (0 == exitCode && exitStatus == QProcess::ExitStatus::NormalExit) {
        retMsg = tr("The tool \"%1\" exited normally.\n").arg(tool.name);
    } else if (exitStatus == QProcess::NormalExit) {
        retMsg = tr("The tool \"%1\" exited with code %2.\n").arg(tool.name, QString::number(exitCode));
    } else {
        retMsg = tr("The tool \"%1\" crashed.\n").arg(tool.name);
    }

    printOutput(id, retMsg, OutputPane::OutputFormat::NormalMessage);
    QString endMsg = tr("Execute tool \"%1\" finished.\n").arg(tool.name);
    printOutput(id, endMsg, OutputPane::OutputFormat::NormalMessage);
    AppOutputPane::instance()->setProcessFinished(id);

    if (toolTaskMap.contains(id)) {
        auto task = toolTaskMap[id];
        std::get<1>(task)->quit();
        std::get<1>(task)->wait();
        toolTaskMap.remove(id);
    }
}

void BinaryToolsManager::handleReadOutput(const QString &id)
{
    if (!toolTaskMap.contains(id))
        return;

    auto task = toolTaskMap[id];
    printOutput(id, std::get<0>(task)->readAllStandardOutput(), OutputPane::StdOut);
}

void BinaryToolsManager::handleReadError(const QString &id)
{
    if (!toolTaskMap.contains(id))
        return;

    auto task = toolTaskMap[id];
    printOutput(id, std::get<0>(task)->readAllStandardError(), OutputPane::StdErr);
}

bool BinaryToolsManager::checkCommandExists(const QString &command)
{
    QProcess process;
    process.start("which", QStringList() << command);
    process.waitForFinished();

    if (process.exitCode() != 0)
        return false;

    return true;
}

void BinaryToolsManager::addToToolBar(const ToolInfo &tool)
{
    auto createAction = [this](const ToolInfo &tool) {
        auto act = new QAction(tool.description, this);
        act->setIconText(tool.icon);
        act->setIcon(QIcon::fromTheme(tool.icon));
        connect(act, &QAction::triggered, this, std::bind(&BinaryToolsManager::executeTool, this, tool.id));

        auto actImpl = new AbstractAction(act, this);
        return actImpl;
    };

    if (!windowSrv)
        windowSrv = dpfGetService(WindowService);

    if (!tool.addToToolbar && actMap.contains(tool.id)) {
        // TODO: remove
    } else if (tool.addToToolbar && !actMap.contains(tool.id)) {
        auto act = createAction(tool);
        actMap.insert(tool.id, act);
        windowSrv->addTopToolItemToRight(act, false);
    } else if (tool.addToToolbar && actMap.contains(tool.id)) {
        auto act = actMap[tool.id];
        auto qAct = act->qAction();

        bool changed = false;
        if (tool.description != qAct->text()) {
            changed = true;
            qAct->setText(tool.description);
        }

        if (tool.icon != qAct->iconText()) {
            changed = true;
            qAct->setIconText(tool.icon);
            qAct->setIcon(QIcon::fromTheme(tool.icon));
        }
        // TODO: update toolbar icon and tooltip
    }
}

void BinaryToolsManager::printOutput(const QString &id, const QString &content, OutputPane::OutputFormat format)
{
    uiController.switchContext(tr("&Application Output"));
    auto outputPane = AppOutputPane::instance()->getOutputPaneById(id);
    QString outputContent = content;
    if (format == OutputPane::OutputFormat::NormalMessage) {
        QTextDocument *doc = outputPane->document();
        QTextBlock tb = doc->lastBlock();
        QString lastLineText = tb.text();
        QString prefix;
        if (lastLineText.isEmpty())
            prefix = "";

        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        outputContent = prefix + time + ":" + content;
    }

    OutputPane::AppendMode mode = OutputPane::AppendMode::Normal;
    outputPane->appendText(outputContent, format, mode);
}

void BinaryToolsManager::stopTool(const QString &id)
{
    if (!toolTaskMap.contains(id))
        return;

    auto task = toolTaskMap[id];
    std::get<0>(task)->stop();
    std::get<1>(task)->quit();
    std::get<1>(task)->wait();
}
