// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeporting.h"
#include "common/util/custompaths.h"

#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>

static QStringList kSrcItemNames{QObject::tr("FilePath"), QObject::tr("CodeRange"),
            QObject::tr("Key"), QObject::tr("Suggestion"), QObject::tr("FileType")};
static QStringList kLibItemNames{QObject::tr("FileName"), QObject::tr("Installed"), QObject::tr("Detail")};
CodePorting::CodePorting(QObject *parent)
    : QObject(parent)
{
    connect(&process, &QProcess::started, [this]() {
        this->updateStatus(kRuning);
        QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
                                   .arg(process.program().split("/").last(), process.arguments().join(" "), process.workingDirectory());
        emit outputInformation(startMsg, OutputPane::OutputFormat::NormalMessage);
    });

    connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        process.setReadChannel(QProcess::StandardOutput);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            auto mode = parseFormat(line);
            emit outputInformation(line, OutputPane::OutputFormat::StdOut, mode);
        }
    });

    connect(&process, &QProcess::readyReadStandardError, [&]() {
        process.setReadChannel(QProcess::StandardError);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            QRegularExpression reg("\\s\\[INFO\\]\\s");
            bool isInfo = reg.match(line).hasMatch();
            OutputPane::OutputFormat format = isInfo ? OutputPane::StdOut : OutputPane::StdErr;
            auto mode = parseFormat(line);
            emit outputInformation(line, format, mode);

            // The output content include report path, so get it.
            QString reportPath = parseReportPath(line);
            if (!reportPath.isEmpty()) {
                bool bSuccessful = parseReportFromFile(reportPath);
                if (bSuccessful) {
                    emit outputInformation(tr("Parse report successful.\n"), OutputPane::StdOut, mode);
                } else {
                    emit outputInformation(tr("Parse report Failed.\n"), OutputPane::StdErr, mode);
                }
            }
        }
    });

    connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
                QString retMsg;
                OutputPane::OutputFormat format = OutputPane::NormalMessage;
                if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
                    retMsg = tr("The process \"%1\" exited normally.\n").arg(process.program());
                    this->updateStatus(kSuccessful);
                } else if (exitStatus == QProcess::NormalExit) {
                    retMsg = tr("The process \"%1\" exited with code %2.\n")
                                     .arg(process.program(), QString::number(exitcode));
                    this->updateStatus(kSuccessful);
                } else {
                    retMsg = tr("The process \"%1\" crashed.\n")
                                     .arg(process.program());
                    format = OutputPane::ErrorMessage;
                    this->updateStatus(kFailed);
                }
                emit outputInformation(retMsg, format);
            });
}

bool CodePorting::start(const QString &projectSrcPath, const QString &srcCPU, const QString &buildDir, const QString &destCPU)
{
    if (status == kRuning)
        return false;

    // check script and source dirctory is exists.
    QString scriptPath = CustomPaths::global(CustomPaths::Scripts);
    QString portingCli = scriptPath + "/porting-script/code_porting.py";
    QDir dir;
    if (!QFile::exists(portingCli) || !dir.exists(projectSrcPath))
        return false;

    projSrcPath = projectSrcPath;

    process.setProgram(getPython());
    QStringList args;
    args.append(portingCli);
    args.append("-S");
    args.append(projectSrcPath);
    args.append("-B");
    args.append(buildDir);
    args.append("--scpu");
    args.append(srcCPU);
    args.append("--dcpu");
    args.append(destCPU);
    process.setArguments(args);
    process.start();
    process.waitForFinished(-1);

    return true;
}

bool CodePorting::abort()
{
    return true;
}

CodePorting::PortingStatus CodePorting::getStatus() const
{
    return status;
}

bool CodePorting::isRunning()
{
    return status == kRuning;
}

const CodePorting::Report &CodePorting::getReport() const
{
    return report;
}

const QStringList &CodePorting::getSrcItemNames() const
{
    return kSrcItemNames;
}

const QStringList &CodePorting::getLibItemNames() const
{
    return kLibItemNames;
}

const QList<QStringList> CodePorting::getSourceReport() const
{
    return report["cppfiles"];
}

const QList<QStringList> CodePorting::getDependLibReport() const
{
    return report["sofiles"];
}

/**
 * @brief findAll
 * @param pattern
 * @param str
 * @param Greedy: find all items matched when Greedy is true.
 * @return matched items.
 */
QList<QString> findAll(QString pattern, QString str, bool Greedy)
{
    QRegExp rxlen(pattern);
    rxlen.setMinimal(Greedy);
    int position = 0;
    QList<QString> strList;
    while (position >= 0) {
        position = rxlen.indexIn(str, position);
        if (position < 0)
            break;
        strList << rxlen.cap(1);
        position += rxlen.matchedLength();
    }
    return strList;
}

/**
 * @brief CodePorting::getPython
 * get the latest version
 * @return
 */
QString CodePorting::getPython()
{
    if (pythonCmd.isEmpty()) {
        QDir dir("/usr/bin");
        QStringList filter { "Python*.*" };
        dir.setNameFilters(filter);
        QStringList pythonList = dir.entryList();

        QString pattern = "((\\d)|(\\d+.\\d+))($|\\s)";
        QStringList versions = findAll(pattern, pythonList.join(" "), true);

        double newVersion = 0;
        for (auto version : versions) {
            double v = version.toDouble();
            if (v > newVersion) {
                newVersion = v;
            }
        }
        pythonCmd = "python" + QString::number(newVersion);
    }
    return pythonCmd;
}

void CodePorting::updateStatus(CodePorting::PortingStatus _status)
{
    status = _status;
    emit notifyPortingStatus(status);
}

bool CodePorting::parseReportFromFile(const QString &reportPath)
{
    bool successful = false;

    bool isReportExists = QFile::exists(reportPath);
    qInfo() << "Report exists: " << isReportExists;

    if (isReportExists) {
        QFile file(reportPath);
        if (file.open(QIODevice::ReadOnly)) {
            report.clear();
            const char *quotes = "\"";
            while (!file.atEnd()) {
                QString line = file.readLine();
                QStringList cols = line.split("\",\"");
                if (cols.length() == kItemsCount) {
                    // remove redundant quotes
                    cols.first().remove(quotes);
                    cols.last().remove(quotes);

                    QString type = cols[kFileType].simplified();
                    if (report.find(type) == report.end()) {
                        report.insert(type, {cols});
                    } else {
                        report[type].push_back(cols);
                    }
                }
            }
            file.close();
            successful = true;
        }
    }
    return successful;
}

QString CodePorting::parseReportPath(const QString &line)
{
    QString reportPath;
    QRegularExpression reg("porting advisor for");
    auto match = reg.match(line);
    if (match.hasMatch()) {
        reg.setPattern("(?<=\\s:\\s)(.*)");
        match = reg.match(line);
        if (match.hasMatch()) {
            reportPath = match.captured();
        }
    }
    return reportPath;
}

OutputPane::AppendMode CodePorting::parseFormat(const QString &line)
{
    OutputPane::AppendMode mode = OutputPane::Normal;
    QRegularExpression reg("Running task:");
    auto match = reg.match(line);
    if (match.hasMatch()) {
        mode = OutputPane::OverWrite;
    }
    return mode;
}
