// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytoolsmanager.h"
#include "mainframe/environmentview.h"
#include "mainframe/binarytoolsdialog.h"

#include "common/util/custompaths.h"
#include "common/util/eventdefinitions.h"
#include "common/util/macroexpander.h"
#include "services/window/windowservice.h"
#include "services/terminal/terminalservice.h"
#include "services/editor/editorservice.h"
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
constexpr char AdvanceObject[] { "advance" };
constexpr char UpdateObject[] { "update" };
constexpr char VersionKey[] { "version" };
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
constexpr char MissingHintKey[] { "missingHint" };
constexpr char InstallCommandKey[] { "installCommand" };
constexpr char ChannelDataKey[] { "channelData" };
constexpr char TriggerEventKey[] { "triggerEvent" };
constexpr char UpdateListKey[] { "list" };

using namespace dpfservice;

QString ToolProcess::readAllStandardOutput()
{
    QMutexLocker lk(&mutex);
    return std::move(stdOut);
}

QString ToolProcess::readAllStandardError()
{
    QMutexLocker lk(&mutex);
    return std::move(stdError);
}

void ToolProcess::start(const QString &id)
{
    if (this->id != id)
        return;

    stdOut.clear();
    stdError.clear();
    process.reset(new QProcess);

    connect(process.data(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, std::bind(&ToolProcess::finished, this, id, std::placeholders::_1, std::placeholders::_2));
    connect(process.data(), &QProcess::readyReadStandardOutput, this, [=] {
        QMutexLocker lk(&mutex);
        stdOut += process->readAllStandardOutput();
        Q_EMIT readyReadStandardOutput(id);
    });
    connect(process.data(), &QProcess::readyReadStandardError, this, [=] {
        QMutexLocker lk(&mutex);
        stdError += process->readAllStandardError();
        Q_EMIT readyReadStandardError(id);
    });

    process->setProgram(program);
    process->setArguments(arguments);
    process->setWorkingDirectory(workingDir);
    process->setProcessEnvironment(environment);

    process->start();
    if (!channelData.isEmpty()) {
        process->write(channelData.toLocal8Bit());
        process->closeWriteChannel();
    }
    process->waitForFinished(-1);
}

void ToolProcess::stop()
{
    if (process && process->state() != QProcess::NotRunning)
        process->kill();
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

BinaryToolsManager::BinaryTools BinaryToolsManager::loadConfig(const QString &conf, QString &version)
{
    QFile file(conf);
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

    BinaryTools tools;
    QJsonObject jsonObj = doc.object();
    QJsonArray groups = jsonObj[GroupObject].toArray();
    version = jsonObj[VersionKey].toString();

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

            AdvancedSettings st;
            auto advance = itemObj[AdvanceObject].toObject();
            st.missingHint = advance[MissingHintKey].toString();
            st.installCommand = advance[InstallCommandKey].toString();
            st.channelData = advance[ChannelDataKey].toString();
            st.triggerEvent = advance[TriggerEventKey].toInt();
            itemInfo.advSettings = st;

            itemList.append(itemInfo);
        }

        tools.insert(groupName, itemList);
    }

    return tools;
}

QMap<QString, QStringList> BinaryToolsManager::updateToolList()
{
    QFile file(":/configure/default_binarytools.json");
    if (!file.open(QIODevice::ReadOnly))
        return {};

    auto doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject jsonObj = doc.object();
    QJsonArray updateArray = jsonObj[UpdateObject].toArray();

    QMap<QString, QStringList> toolList;
    for (const QJsonValue &val : updateArray) {
        auto valObj = val.toObject();
        auto version = valObj[VersionKey].toString();
        auto idArray = valObj[UpdateListKey].toArray();
        for (const auto &id : idArray) {
            toolList[version].append(id.toString());
        }
    }

    return toolList;
}

BinaryToolsManager::BinaryTools BinaryToolsManager::mergeTools(const BinaryTools &defTools, const BinaryTools &localTools, const QString &localConfVersion)
{
    const auto &updateList = updateToolList();
    if (updateList.isEmpty())
        return localTools;

    auto tools = std::move(localTools);
    auto iter = updateList.begin();
    for (; iter != updateList.end(); ++iter) {
        if (localConfVersion >= iter.key())
            continue;

        for (const auto &id : iter.value()) {
            const auto &tool = findTool(id, defTools);
            if (!tool.isValid())
                continue;

            if (tools.contains(tool.displyGroup)) {
                tools[tool.displyGroup].append(tool);
                continue;
            }

            tools.insert(tool.displyGroup, QList<ToolInfo>() << tool);
        }
    }

    return tools;
}

QSharedPointer<ToolProcess> BinaryToolsManager::createToolProcess(const ToolInfo &tool)
{
    if (toolTaskMap.contains(tool.id))
        return nullptr;

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

            QJsonObject advObject;
            advObject[MissingHintKey] = item.advSettings.missingHint;
            advObject[InstallCommandKey] = item.advSettings.installCommand;
            advObject[ChannelDataKey] = item.advSettings.channelData;
            advObject[TriggerEventKey] = item.advSettings.triggerEvent;
            itemObject[AdvanceObject] = advObject;

            itemsArray.append(itemObject);
        }

        groupObject[ToolObject] = itemsArray;
        groupsArray.append(groupObject);
    }

    QJsonObject obj;
    obj[GroupObject] = groupsArray;
    obj[VersionKey] = cfgVersion;

    QJsonDocument doc(obj);
    QString confPath = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("binarytools.json");
    QFile file(confPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void BinaryToolsManager::setTools(const BinaryTools &dataMap)
{
    allTools = dataMap;
}

ToolInfo BinaryToolsManager::findTool(const QString &id)
{
    return findTool(id, allTools);
}

ToolInfo BinaryToolsManager::findTool(const QString &id, const BinaryTools &tools)
{
    auto iter = tools.begin();
    for (; iter != tools.end(); ++iter) {
        auto &list = iter.value();
        auto result = std::find_if(list.begin(), list.end(), [&id](const ToolInfo &tool) {
            return tool.id == id;
        });

        if (result != list.end())
            return ToolInfo(*result);
    }

    return {};
}

BinaryToolsManager::BinaryTools BinaryToolsManager::tools()
{
    if (!allTools.isEmpty())
        return allTools;

    QString defCfg = ":/configure/default_binarytools.json";
    const auto &defTools = loadConfig(defCfg, cfgVersion);
    QString localCfg = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("binarytools.json");
    if (!QFile::exists(localCfg)) {
        allTools = std::move(defTools);
        return allTools;
    }

    QString localCfgVersion;
    allTools = loadConfig(localCfg, localCfgVersion);
    if (cfgVersion > localCfgVersion)
        allTools = mergeTools(defTools, allTools, localCfgVersion);

    return allTools;
}

void BinaryToolsManager::executeTool(const QString &id)
{
    const auto &tool = this->findTool(id);
    if (!tool.isValid())
        return;

    if (!checkCommandExists(tool.command))
        return toolMissingHint(tool);

    AppOutputPane::instance()->createApplicationPane(id, tool.name);
    auto stopHandler = std::bind(&BinaryToolsManager::stopTool, this, id);
    AppOutputPane::instance()->setStopHandler(id, stopHandler);
    QString startMsg = tr("Start execute tool \"%1\".\n").arg(tool.name);
    printOutput(id, startMsg, OutputPane::NormalMessage);
    uiController.switchContext(tr("&Application Output"));

    auto toolProcess = createToolProcess(tool);
    if (!toolProcess) {
        printOutput(id, tr("The tool is running. Please stop it before running.\n"), OutputPane::ErrorMessage);
        return;
    }

    QString errorMsg;
    if (!checkAndSetProcessParams(toolProcess, tool, errorMsg)) {
        printOutput(id, errorMsg, OutputPane::ErrorMessage);
        stopTool(id);
        toolTaskMap.remove(id);
        AppOutputPane::instance()->setProcessFinished(id);
        return;
    }

    Q_EMIT execute(id);
}

void BinaryToolsManager::checkAndAddToToolbar(const BinaryTools &tools)
{
    auto iter = tools.begin();
    for (; iter != tools.end(); ++iter) {
        for (const auto &tool : iter.value())
            addToToolBar(tool);
    }
}

void BinaryToolsManager::updateToolMenu(const BinaryTools &tools)
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

void BinaryToolsManager::installTool(const QString &id)
{
    const auto &tool = findTool(id);
    if (!tool.isValid())
        return;

    if (!terminalSrv)
        terminalSrv = dpfGetService(TerminalService);

    uiController.switchContext(TERMINAL_TAB_TEXT);
    terminalSrv->sendCommand(tool.advSettings.installCommand);
}

void BinaryToolsManager::eventTriggered(EventType event, const QVariantList &args)
{
    Q_UNUSED(args)

    auto iter = allTools.begin();
    for (; iter != allTools.end(); ++iter) {
        auto &list = iter.value();
        for (const auto &tool : list) {
            if (tool.advSettings.triggerEvent != event)
                continue;

            executeTool(tool.id);
        }
    }
}

void BinaryToolsManager::executeFinished(const QString &id, int exitCode, QProcess::ExitStatus exitStatus)
{
    const auto &tool = findTool(id);
    if (!tool.isValid())
        return;

    QString retMsg;
    if (0 == exitCode && exitStatus == QProcess::ExitStatus::NormalExit) {
        if (tool.outputOption == ReplaceCurrentDocument)
            replaceCurrentDocument(id, exitCode);

        retMsg = tr("The tool \"%1\" exited normally.\n").arg(tool.name);
    } else if (exitStatus == QProcess::NormalExit) {
        if (tool.errorOutputOption == ReplaceCurrentDocument)
            replaceCurrentDocument(id, exitCode);

        retMsg = tr("The tool \"%1\" exited with code %2.\n").arg(tool.name, QString::number(exitCode));
    } else {
        retMsg = tr("The tool \"%1\" crashed.\n").arg(tool.name);
    }

    uiController.switchContext(tr("&Application Output"));
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
    auto stdOut = std::get<0>(task)->readAllStandardOutput();
    printOutput(id, stdOut, OutputPane::StdOut);
}

void BinaryToolsManager::handleReadError(const QString &id)
{
    if (!toolTaskMap.contains(id))
        return;

    auto task = toolTaskMap[id];
    auto stdError = std::get<0>(task)->readAllStandardError();
    printOutput(id, stdError, OutputPane::StdErr);
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

void BinaryToolsManager::toolMissingHint(const ToolInfo &tool)
{
    if (!windowSrv)
        windowSrv = dpfGetService(WindowService);

    const auto &st = tool.advSettings;
    QString msg = st.missingHint;
    if (msg.isEmpty())
        msg = tr("The tool (%1) execution program does not exist. Install and run it again").arg(tool.name);

    QStringList actions { "ok_default", tr("Ok") };
    if (!st.installCommand.isEmpty()) {
        actions.clear();
        actions << "cancel"
                << tr("Cancel")
                << tool.id + "_install_default"
                << tr("Install");
    }

    windowSrv->notify(2, "", msg, actions);
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
        windowSrv->removeTopToolItem(actMap[tool.id]);
        actMap.remove(tool.id);
    } else if (tool.addToToolbar && !actMap.contains(tool.id)) {
        auto act = createAction(tool);
        actMap.insert(tool.id, act);
        windowSrv->addTopToolItemToRight(act, false, Priority::high);
    } else if (tool.addToToolbar && actMap.contains(tool.id)) {
        auto act = actMap[tool.id];
        auto qAct = act->qAction();

        if (tool.description != qAct->text()) {
            qAct->setText(tool.description);
        }

        if (tool.icon != qAct->iconText()) {
            qAct->setIconText(tool.icon);
            qAct->setIcon(QIcon::fromTheme(tool.icon));
        }
    }
}

void BinaryToolsManager::printOutput(const QString &id, const QString &content, OutputPane::OutputFormat format)
{
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

void BinaryToolsManager::replaceCurrentDocument(const QString &id, int exitCode)
{
    if (!toolTaskMap.contains(id))
        return;

    QString text;
    if (exitCode == 0)
        text = std::get<0>(toolTaskMap[id])->readAllStandardOutput();
    else
        text = std::get<0>(toolTaskMap[id])->readAllStandardError();

    if (!editorSrv)
        editorSrv = dpfGetService(EditorService);

    if (!text.isEmpty())
        editorSrv->setText(text);
}

bool BinaryToolsManager::checkAndSetProcessParams(QSharedPointer<ToolProcess> process, const ToolInfo &tool, QString &errorMsg)
{
    process->setId(tool.id);
    process->setProgram(tool.command);
    auto args = globalMacroExpander()->expandArguments(tool.arguments);
    QStringList argList = args.split(" ", QString::SkipEmptyParts);
    process->setArguments(argList);

    if (!tool.workingDirectory.isEmpty()) {
        auto workingDir = globalMacroExpander()->expand(tool.workingDirectory);
        if (workingDir.isEmpty()) {
            errorMsg = tr("The tool has set the working directory, but the working directory parsing is empty. Please check and try again.\n");
            return false;
        }
        process->setWorkingDirectory(workingDir);
    }

    if (!tool.advSettings.channelData.isEmpty()) {
        auto channelData = globalMacroExpander()->expand(tool.advSettings.channelData);
        if (channelData.isEmpty()) {
            errorMsg = tr("The tool has set the channel data, but the channel data parsing is empty. Please check and try again.\n");
            return false;
        }
        process->setChannelData(channelData);
    }

    QProcessEnvironment env;
    auto iterator = tool.environment.begin();
    while (iterator != tool.environment.end()) {
        env.insert(iterator.key(), iterator.value().toString());
        ++iterator;
    }
    process->setProcessEnvironment(env);

    return true;
}
