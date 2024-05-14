// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytoolsmanager.h"
#include "mainframe/environmentview.h"

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

    QProcess proc;
    QString retMsg = tr("Error: execute command error! The reason is unknown.\n");
    connect(&proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
                if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
                    retMsg = tr("The process \"%1\" exited normally.\n").arg(proc.program());
                } else if (exitStatus == QProcess::NormalExit) {
                    retMsg = tr("The process \"%1\" exited with code %2.\n")
                                     .arg(proc.program(), QString::number(exitcode));
                } else {
                    retMsg = tr("The process \"%1\" crashed.\n").arg(proc.program());
                }
            });

    if (tool.errorOutputOption == ShowInApplicationOutput) {
        connect(&proc, &QProcess::readyReadStandardError, this, [&] {
            proc.setReadChannel(QProcess::StandardError);
            while (proc.canReadLine()) {
                QString line = QString::fromUtf8(proc.readLine());
                qInfo() << line;
                printOutput(line, OutputPane::OutputFormat::StdErr);
            }
        });
    }

    if (tool.outputOption == ShowInApplicationOutput) {
        connect(&proc, &QProcess::readyReadStandardOutput, this, [&] {
            proc.setReadChannel(QProcess::StandardOutput);
            while (proc.canReadLine()) {
                QString line = QString::fromUtf8(proc.readLine());
                qInfo() << line;
                printOutput(line, OutputPane::OutputFormat::StdOut);
            }
        });
    }

    QStringList argList = tool.arguments.split(" ", QString::SkipEmptyParts);
    proc.setProgram(tool.command);
    proc.setArguments(argList);
    proc.setWorkingDirectory(tool.workingDirectory);
    QProcessEnvironment env;
    auto iterator = tool.environment.begin();
    while (iterator != tool.environment.end()) {
        env.insert(iterator.key(), iterator.value().toString());
        ++iterator;
    }
    proc.setProcessEnvironment(env);

    QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
                               .arg(tool.command, tool.arguments, tool.workingDirectory);
    printOutput(startMsg, OutputPane::OutputFormat::NormalMessage);
    proc.startDetached();
    proc.waitForFinished(-1);

    printOutput(retMsg, OutputPane::OutputFormat::NormalMessage);
    QString endMsg = tr("Execute command finished.\n");
    printOutput(endMsg, OutputPane::OutputFormat::NormalMessage);
}

void BinaryToolsManager::checkAndAddToToolbar(const QMap<QString, QList<ToolInfo>> &tools)
{
    auto iter = tools.begin();
    for (; iter != tools.end(); ++iter) {
        for (const auto &tool : iter.value())
            addToToolBar(tool);
    }
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
        auto act = new QAction(this);
        act->setIcon(QIcon::fromTheme(tool.icon));
        connect(act, &QAction::triggered, this, std::bind(&BinaryToolsManager::executeTool, this, tool.id));

        auto actImpl = new AbstractAction(act, this);
        actImpl->setShortCutInfo(tool.id, tool.description);
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
    }
}

void BinaryToolsManager::printOutput(const QString &content, OutputPane::OutputFormat format)
{
    uiController.switchContext(tr("&Application Output"));
    auto outputPane = OutputPane::instance();
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
