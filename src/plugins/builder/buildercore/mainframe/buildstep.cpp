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
#include "transceiver/eventsender.h"
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
    bool ret = true;

    process.reset(new QProcess());
    process->setWorkingDirectory(buildOutputDir);

    connect(process.get(), static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitCode, QProcess::ExitStatus status) {
        if (status == QProcess::NormalExit && exitCode == 0) {
            emit addOutput(tr("The process \"%1\" exited normally.").arg(process->program()),
                           OutputFormat::NormalMessage);
        } else if (status == QProcess::NormalExit) {
            ret = false;
            emit addOutput(tr("The process \"%1\" exited with code %2.")
                           .arg(process->program(), QString::number(exitCode)),
                           OutputFormat::ErrorMessage);
        } else {
            ret = false;
            emit addOutput(tr("The process \"%1\" crashed.").arg(process->program()), OutputFormat::ErrorMessage);
        }
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

    return ret;
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
