// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcreplacehworker.h"
#include "searchplaceworker_p.h"

#include "common/util/qtcassert.h"

#include <QProcess>
#include <QMutex>
#include <QSharedPointer>
#include <QRegularExpression>

using namespace dpfservice;

SearchReplaceWorkerPrivate::SearchReplaceWorkerPrivate(SearchReplaceWorker *qq)
    : q(qq)
{
}

SearchReplaceWorkerPrivate::~SearchReplaceWorkerPrivate()
{
    q->stop();
}

void SearchReplaceWorkerPrivate::startAll()
{
    QTC_ASSERT(!jobList.isEmpty(), return );
    QTC_ASSERT(!process, return );

    currentJob = 0;
    startNextJob();
}

void SearchReplaceWorkerPrivate::startNextJob()
{
    QTC_ASSERT(currentJob < jobList.count(), return );

    Job job = jobList.at(currentJob);
    process.reset(new QProcess);
    if (job.type == SearchJob)
        connect(process.data(), &QProcess::readyReadStandardOutput, q,
                std::bind(&SearchReplaceWorker::handleReadSearchResult, q, job.keyword, job.caseSensitive, job.wholeWords));
    connect(process.data(), qOverload<int>(&QProcess::finished),
            q, std::bind(&SearchReplaceWorker::processDone, q, job.type));

    process->start(job.cmd);
    if (!job.channelData.isEmpty()) {
        process->write(job.channelData.toUtf8());
        process->closeWriteChannel();
    }
    process->waitForFinished(-1);
}

void SearchReplaceWorkerPrivate::createSearchJob(const SearchParams &params)
{
    jobList.clear();
    switch (params.scope) {
    case AllProjects:
    case CurrentProject: {
        auto tmpParams = params;
        processWorkingFiles(tmpParams.baseParams.baseFileList, tmpParams.baseParams.openedFileList);
        for (const auto &file : tmpParams.baseParams.openedFileList) {
            const auto &job = buildSearchJob({ file }, tmpParams.includeList, tmpParams.excludeList,
                                             tmpParams.baseParams.keyword, tmpParams.flags, true);
            if (!job.cmd.isEmpty())
                jobList << job;
        }

        const auto &job = buildSearchJob(tmpParams.baseParams.baseFileList, tmpParams.includeList, tmpParams.excludeList,
                                         tmpParams.baseParams.keyword, tmpParams.flags, false);
        if (!job.cmd.isEmpty())
            jobList << job;
    } break;
    case CurrentFile: {
        const auto &job = buildSearchJob(params.baseParams.openedFileList, params.includeList, params.excludeList,
                                         params.baseParams.keyword, params.flags, true);
        if (!job.cmd.isEmpty())
            jobList << job;
    } break;
    default:
        break;
    }
}

void SearchReplaceWorkerPrivate::createReplaceJob(const ReplaceParams &params)
{
    jobList.clear();
    auto tmpParams = params;
    processWorkingFiles(tmpParams.baseParams.baseFileList, tmpParams.baseParams.openedFileList);
    buildReplaceJob(tmpParams.baseParams.openedFileList,
                    tmpParams.baseParams.keyword,
                    tmpParams.replaceText,
                    tmpParams.flags, true);

    const auto &job = buildReplaceJob(tmpParams.baseParams.baseFileList,
                                      tmpParams.baseParams.keyword,
                                      tmpParams.replaceText,
                                      tmpParams.flags, false);
    if (!job.cmd.isEmpty())
        jobList << job;
}

SearchReplaceWorkerPrivate::Job
SearchReplaceWorkerPrivate::buildSearchJob(const QStringList &fileList,
                                           const QStringList &includeList,
                                           const QStringList &excludeList,
                                           const QString &keyword, SearchFlags flags,
                                           bool isOpenedFile)
{
    if (fileList.isEmpty())
        return {};

    Job job;
    job.type = SearchJob;
    job.keyword = keyword;
    QStringList cmd;
    cmd << "grep -Hn";
    if (!flags.testFlag(SearchCaseSensitive))
        cmd << "-i";
    else
        job.caseSensitive = true;

    if (flags.testFlag(SearchWholeWord)) {
        cmd << "-w";
        job.wholeWords = true;
    }

    if (!includeList.isEmpty())
        cmd << "--include=" + includeList.join(" --include=");
    if (!excludeList.isEmpty())
        cmd << "--exclude=" + excludeList.join(" --exclude=");
    cmd << "\"" + keyword + "\"";

    if (isOpenedFile) {
        if (!editSrv)
            editSrv = dpfGetService(EditorService);
        job.channelData = editSrv->fileText(fileList.first());
        cmd << "--label=" + fileList.first();
    } else {
        QString searchPath = fileList.join(' ');
        cmd << searchPath;
    }

    job.cmd = cmd.join(' ');
    return job;
}

SearchReplaceWorkerPrivate::Job
SearchReplaceWorkerPrivate::buildReplaceJob(const QStringList &fileList,
                                            const QString &oldText,
                                            const QString &newText,
                                            SearchFlags flags,
                                            bool isOpenedFile)
{
    if (fileList.isEmpty())
        return {};

    Job job;
    job.type = ReplaceJob;
    if (isOpenedFile) {
        for (const auto &file : fileList) {
            QMetaObject::invokeMethod(this, "replaceOpenedFile",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, file),
                                      Q_ARG(QString, oldText),
                                      Q_ARG(QString, newText),
                                      Q_ARG(bool, flags.testFlag(SearchCaseSensitive)),
                                      Q_ARG(bool, flags.testFlag(SearchWholeWord)));
        }
    } else {
        QStringList cmd;
        cmd << "sed -i ";

        QString body("s/%1/%2/g");
        if (flags.testFlag(SearchWholeWord)) {
            body = body.arg("\\<" + oldText + "\\>", newText);
        } else {
            body = body.arg(oldText, newText);
        }

        if (!flags.testFlag(SearchCaseSensitive))
            body += "I";

        cmd << "\"" + body + "\""
            << fileList.join(' ');
        job.cmd = cmd.join(' ');
    }

    return job;
}

void SearchReplaceWorkerPrivate::processWorkingFiles(QStringList &baseFiles, QStringList &openedFiles)
{
    for (int i = 0; i < openedFiles.size();) {
        if (!baseFiles.contains(openedFiles.at(i))) {
            openedFiles.removeAt(i);
        } else {
            baseFiles.removeOne(openedFiles.at(i));
            ++i;
        }
    }
}

void SearchReplaceWorkerPrivate::replaceOpenedFile(const QString &fileName, const QString &oldText,
                                                   const QString &newText, bool caseSensitive, bool wholeWords)
{
    if (!editSrv)
        editSrv = dpfGetService(EditorService);
    editSrv->replaceAll(fileName, oldText, newText, caseSensitive, wholeWords);
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
    if (d->process) {
        d->process->kill();
        d->process->waitForFinished();
        d->process.reset();
    }
}

FindItemList SearchReplaceWorker::getResults()
{
    QMutexLocker lk(&d->mutex);
    return std::move(d->searchResults);
}

void SearchReplaceWorker::addSearchTask(const SearchParams &params)
{
    d->createSearchJob(params);
    if (d->jobList.isEmpty()) {
        Q_EMIT searchFinished();
        return;
    }

    d->startAll();
}

void SearchReplaceWorker::addReplaceTask(const ReplaceParams &params)
{
    d->createReplaceJob(params);
    if (d->jobList.isEmpty()) {
        Q_EMIT replaceFinished(0);
        return;
    }

    d->startAll();
}

void SearchReplaceWorker::handleReadSearchResult(const QString &keyword, bool caseSensitive, bool wholeWords)
{
    const auto resultLineRegex = QRegularExpression(R"((.+):([0-9]+):(.+))", QRegularExpression::NoPatternOption);
    while (d->process->canReadLine()) {
        const auto &line = d->process->readLine();
        QRegularExpressionMatch regMatch;
        if ((regMatch = resultLineRegex.match(line)).hasMatch()) {
            auto name = regMatch.captured(1);
            auto line = regMatch.captured(2).toInt();
            auto context = regMatch.captured(3);

            QString pattern = keyword;
            if (wholeWords)
                pattern = "\\b" + keyword + "\\b";
            QRegularExpression regex(pattern, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
            for (int index = context.indexOf(regex); index != -1;) {
                FindItem findItem;
                findItem.filePathName = name;
                findItem.line = line;
                findItem.keyword = keyword;
                findItem.context = context;
                findItem.column = index;

                index = context.indexOf(regex, index + keyword.size());
                QMutexLocker lk(&d->mutex);
                d->searchResults.append(findItem);
            }
        }
    }

    QMutexLocker lk(&d->mutex);
    if (!d->searchResults.isEmpty())
        Q_EMIT matched();
}

void SearchReplaceWorker::processDone(int jobType)
{
    ++d->currentJob;
    const bool success = d->process->exitCode() == 0;
    if (d->currentJob < d->jobList.count() && success) {
        d->process.reset();
        d->startNextJob();
        return;
    }

    if (jobType == SearchReplaceWorkerPrivate::SearchJob)
        Q_EMIT searchFinished();
    else
        Q_EMIT replaceFinished(d->process->exitCode());

    d->process.reset();
}
