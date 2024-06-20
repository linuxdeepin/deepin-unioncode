// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcreplacehworker.h"

#include <QProcess>
#include <QMutex>
#include <QSharedPointer>
#include <QRegularExpression>

class SearchReplaceWorkerPrivate
{
public:
    explicit SearchReplaceWorkerPrivate(SearchReplaceWorker *qq);
    ~SearchReplaceWorkerPrivate();

    QString buildCommand(const SearchParams &params);
    QString buildCommand(const ReplaceParams &params);

public:
    SearchReplaceWorker *q;

    QMutex mutex;
    FindItemList searchResults;
    QAtomicInteger<bool> isRuning { false };
    QSharedPointer<QProcess> searchProcess { nullptr };
    QSharedPointer<QProcess> replaceProcess { nullptr };
    QString filePath;
};

SearchReplaceWorkerPrivate::SearchReplaceWorkerPrivate(SearchReplaceWorker *qq)
    : q(qq)
{
}

SearchReplaceWorkerPrivate::~SearchReplaceWorkerPrivate()
{
    if (searchProcess && searchProcess->state() != QProcess::NotRunning)
        searchProcess->kill();

    if (replaceProcess && replaceProcess->state() != QProcess::NotRunning)
        replaceProcess->kill();
}

QString SearchReplaceWorkerPrivate::buildCommand(const SearchParams &params)
{
    filePath = params.filePathList.join(" ");
    QString sensitiveFlag = params.sensitiveFlag ? "" : " -i ";
    QString wholeWordsFlag = params.wholeWordsFlag ? " -w " : "";
    QString patternList;
    if (!params.patternsList.isEmpty())
        patternList = " --include=" + params.patternsList.join(" --include=");

    QString exPatternList(" --exclude-dir=.*");
    if (!params.exPatternsList.isEmpty())
        exPatternList += " --exclude={" + params.exPatternsList.join(",") + "}";

    QString cmd = QString("grep -rn " + sensitiveFlag + wholeWordsFlag
                          + "\"" + params.searchText + "\" "
                          + patternList + exPatternList + " " + filePath);
    return cmd;
}

QString SearchReplaceWorkerPrivate::buildCommand(const ReplaceParams &params)
{
    QString filePath = params.filePathList.join(' ');

    //exam: sed -i "s/main/main1/g" `grep -rl "main" /project/test`
    QString cmd = "sed -i \"s/" + params.searchText
            + "/" + params.replaceText + "/g\" `grep -rl \"" + params.searchText
            + "\" " + filePath + "`";
    return cmd;
}

SearchReplaceWorker::SearchReplaceWorker(QObject *parent)
    : QObject(parent),
      d(new SearchReplaceWorkerPrivate(this))
{
}

SearchReplaceWorker::~SearchReplaceWorker()
{
    delete d;
}

void SearchReplaceWorker::stop()
{
    d->isRuning = false;
}

FindItemList SearchReplaceWorker::getResults()
{
    QMutexLocker lk(&d->mutex);
    return std::move(d->searchResults);
}

void SearchReplaceWorker::addSearchTask(const SearchParams &params)
{
    d->isRuning = true;
    if (!d->searchProcess)
        d->searchProcess.reset(new QProcess);

    connect(d->searchProcess.data(), &QProcess::readyReadStandardOutput,
            this, &SearchReplaceWorker::handleReadSearchResult,
            Qt::UniqueConnection);
    connect(d->searchProcess.data(), qOverload<int>(&QProcess::finished),
            this, &SearchReplaceWorker::searchFinished,
            Qt::UniqueConnection);

    const auto &cmd = d->buildCommand(params);
    d->searchProcess->start(cmd);
    d->searchProcess->waitForFinished(-1);
    d->isRuning = false;
}

void SearchReplaceWorker::addReplaceTask(const ReplaceParams &params)
{
    d->isRuning = true;
    if (!d->replaceProcess)
        d->replaceProcess.reset(new QProcess);

    connect(d->replaceProcess.data(), qOverload<int>(&QProcess::finished),
            this, &SearchReplaceWorker::replaceFinished,
            Qt::UniqueConnection);

    const auto &cmd = d->buildCommand(params);
    QStringList options;
    options << "-c" << cmd;

    d->replaceProcess->start("/bin/sh", options);
    d->replaceProcess->waitForFinished(-1);
    d->isRuning = false;
}

void SearchReplaceWorker::handleReadSearchResult()
{
    if (!d->isRuning) {
        d->searchProcess->terminate();
        return;
    }

    const auto resultLineRegex = QRegularExpression(R"((.+):([0-9]+):(.+))", QRegularExpression::NoPatternOption);
    while (d->searchProcess->canReadLine()) {
        const auto &line = d->searchProcess->readLine();
        if (!d->isRuning) {
            d->searchProcess->terminate();
            return;
        }

        QRegularExpressionMatch regMatch;
        if ((regMatch = resultLineRegex.match(line)).hasMatch()) {
            FindItem findItem;
            findItem.filePathName = regMatch.captured(1).trimmed().toStdString().c_str();
            findItem.lineNumber = regMatch.captured(2).trimmed().toInt();
            findItem.context = regMatch.captured(3).trimmed().toStdString().c_str();

            QMutexLocker lk(&d->mutex);
            d->searchResults.append(findItem);
        }
    }

    QMutexLocker lk(&d->mutex);
    if (!d->searchResults.isEmpty())
        Q_EMIT matched();
}
