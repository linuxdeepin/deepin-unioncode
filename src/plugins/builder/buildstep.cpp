/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "buildstep.h"
#include "eventsender.h"
#include "tasks/ansifilterparser.h"
#include "common/util/qtcassert.h"

#include <QDir>
#include <QDebug>

/*
 *  Parser not implemented, use macro to be holder.
 */
//#define PARSE(O)

BuildStep::BuildStep(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<OutputFormat>("OutputFormat");
}

void BuildStep::setToolChainType(ToolChainType type)
{
    toolChainType = type;
}

void BuildStep::setBuildOutputDir(const QString &outputDir)
{
    buildOutputDir = outputDir;
}

void BuildStep::setMakeFile(const QString &filePath)
{
    makeFile = filePath;
}

void BuildStep::appendCmdParam(const QString param)
{
    cmdParams << param;
}

void BuildStep::setOutputParser(IOutputParser *parser)
{
    outputParserChain.reset(new AnsiFilterParser);
    outputParserChain->appendOutputParser(parser);

//    connect(m_outputParserChain.get(), &IOutputParser::addOutput, this, &BuildStep::outputAdded);
    connect(outputParserChain.get(), &IOutputParser::addTask, this, &BuildStep::taskAdded);
}

void BuildStep::appendOutputParser(IOutputParser *parser)
{
    if (!parser)
        return;

    QTC_ASSERT(outputParserChain, return);
    outputParserChain->appendOutputParser(parser);
}

IOutputParser *BuildStep::outputParser() const
{
    return outputParserChain.get();
}

void BuildStep::stdOutput(const QString &line)
{
    // Get target infomation.
    parse(line);

    if (outputParserChain)
        outputParserChain->stdOutput(line);

    emit addOutput(line, OutputFormat::Stdout);
}

void BuildStep::stdErrput(const QString &line)
{
    if (outputParserChain)
        outputParserChain->stdError(line);

    emit addOutput(line, OutputFormat::Stderr);
}

bool BuildStep::execCmd(const QString &cmd, const QStringList &args)
{
    bool ret = false;
    if (buildOutputDir.isEmpty()) {
        for (auto arg : args) {
            QFileInfo fileInfo(arg);
            if (fileInfo.isDir()) {
                buildOutputDir = arg;
                break;
            }
        }
    }
    QDir outputDir(buildOutputDir);
    if (outputDir.exists()) {
        process.reset(new QProcess());
        process->setWorkingDirectory(buildOutputDir);

        connect(process.get(), static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
                [&](int, QProcess::ExitStatus) {
            if (!buildOutputDir.isEmpty() && !targetName.isEmpty()) {
                QString fullPath = buildOutputDir + "/" + targetName;
                EventSender::notifyTargetPath(fullPath);
            }
            qDebug() << "build step finished";
        });

        connect(process.get(), &QProcess::started,
                [&](){
            qDebug() << "build step starting";
        });

        connect(process.get(), &QProcess::readyReadStandardOutput,
                this, &BuildStep::processReadyReadStdOutput);

        connect(process.get(), &QProcess::readyReadStandardError,
                this, &BuildStep::processReadyReadStdError);

        // TODO(mozart) : should output more message here.

        QStringList params;
        process->start(cmd, args);
        process->waitForFinished();

        if (process->exitStatus() == QProcess::ExitStatus::NormalExit)
            ret = true;
    }
    return ret;
}

void BuildStep::parse(const QString &line)
{
    auto formateString = [](QString &str){
        str.remove(QChar('\n'));
        str.remove(QChar(' '));
    };

    if (line.contains("Build files have been written to:")) {
        QStringList items = line.split(":");
        buildOutputDir = items.last();
        formateString(buildOutputDir);
        qInfo() << "----Buildoutputpath:" <<buildOutputDir;
    }

    if (line.contains("Built target") && !line.contains("autogen")) {
        QStringList items = line.split("Built target ");
        targetName = items.last();
        formateString(targetName);
        qInfo() << "----Built target:" << targetName;
    }
}

void BuildStep::taskAdded(const Task &task, int linkedOutputLines, int skipLines)
{
    emit addTask(task, linkedOutputLines, skipLines);
}

void BuildStep::processReadyReadStdOutput()
{
    process->setReadChannel(QProcess::StandardOutput);
    while (process->canReadLine()) {
        QString line = QString::fromUtf8(process->readLine());
        stdOutput(line);
    }
}

void BuildStep::processReadyReadStdError()
{
    process->setReadChannel(QProcess::StandardError);
    while (process->canReadLine()) {
        QString line = QString::fromUtf8(process->readLine());
        stdErrput(line);
    }
}
