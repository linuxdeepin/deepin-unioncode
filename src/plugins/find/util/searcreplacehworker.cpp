// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcreplacehworker.h"

#include "common/util/qtcassert.h"
#include "services/editor/editorservice.h"

#include <QProcess>
#include <QMutex>
#include <QSharedPointer>
#include <QRegularExpression>

using namespace dpfservice;

class SearchReplaceWorkerPrivate : public QObject
{
public:
    enum JobType {
        SearchJob,
        ReplaceJob
    };

    struct Job
    {
        JobType type;
        QString cmd;
        QString channelData;
    };

    explicit SearchReplaceWorkerPrivate(SearchReplaceWorker *qq);
    ~SearchReplaceWorkerPrivate();

    void startAll();
    void startNextJob();

    void createSearchJob(const SearchParams &params);
    Job buildSearchJob(const QStringList &fileList,
                       const QStringList &includeList,
                       const QStringList &excludeList,
                       const QString &keyword, SearchFlags flags,
                       bool isOpenedFile);
    QString buildCommand(const ReplaceParams &params);

    void processWorkingFiles(QStringList &projectFiles, QStringList &openedFiles);

public:
    SearchReplaceWorker *q;

    QMutex mutex;
    FindItemList searchResults;
    QSharedPointer<QProcess> process { nullptr };
    QSharedPointer<QProcess> replaceProcess { nullptr };

    QList<Job> jobList;
    int currentJob = 0;
};

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
        connect(process.data(), &QProcess::readyReadStandardOutput, q, &SearchReplaceWorker::handleReadSearchResult);
    connect(process.data(), qOverload<int>(&QProcess::finished),
            q, std::bind(&SearchReplaceWorker::processDone, q, job.type));

    process->start(job.cmd);
    if (!job.channelData.isEmpty()) {
        process->write(job.channelData.toUtf8());
        process->closeWriteChannel();
    }
}

void SearchReplaceWorkerPrivate::createSearchJob(const SearchParams &params)
{
    jobList.clear();
    switch (params.baseParams.scope) {
    case AllProjects:
    case CurrentProject: {
        auto tmpParams = params;
        processWorkingFiles(tmpParams.baseParams.projectFileList, tmpParams.baseParams.openedFileList);
        for (const auto &file : tmpParams.baseParams.openedFileList) {
            const auto &job = buildSearchJob({ file }, tmpParams.includeList, tmpParams.excludeList,
                                             tmpParams.baseParams.keyword, tmpParams.flags, true);
            if (!job.cmd.isEmpty())
                jobList << job;
        }

        const auto &job = buildSearchJob(tmpParams.baseParams.projectFileList, tmpParams.includeList, tmpParams.excludeList,
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

SearchReplaceWorkerPrivate::Job SearchReplaceWorkerPrivate::buildSearchJob(const QStringList &fileList,
                                                                           const QStringList &includeList,
                                                                           const QStringList &excludeList,
                                                                           const QString &keyword, SearchFlags flags,
                                                                           bool isOpenedFile)
{
    if (fileList.isEmpty())
        return {};

    Job job;
    job.type = SearchJob;
    QStringList cmd;
    cmd << "grep -Hn";
    if (!flags.testFlag(SearchCaseSensitive))
        cmd << "-i";
    if (flags.testFlag(SearchWholeWord))
        cmd << "-w";
    if (!includeList.isEmpty())
        cmd << "--include=" + includeList.join(" --include=");
    if (!excludeList.isEmpty())
        cmd << "--exclude=" + excludeList.join(" --exclude=");
    cmd << "\"" + keyword + "\"";

    if (isOpenedFile) {
        auto editSrv = dpfGetService(EditorService);
        job.channelData = editSrv->fileText(fileList.first());
        cmd << "--label=" + fileList.first();
    } else {
        QString searchPath = fileList.join(' ');
        cmd << searchPath;
    }

    job.cmd = cmd.join(' ');
    return job;
}

QString SearchReplaceWorkerPrivate::buildCommand(const ReplaceParams &params)
{
    QString filePath = params.filePathList.join(' ');

    //exam: sed -i "s/main/main1/g" `grep -rl "main" /project/test`
    QString cmd = "sed -i \"s/" + params.keyword
            + "/" + params.replaceText + "/g\" `grep -rl \"" + params.keyword
            + "\" " + filePath + "`";
    return cmd;
}

void SearchReplaceWorkerPrivate::processWorkingFiles(QStringList &projectFiles, QStringList &openedFiles)
{
    for (int i = 0; i < openedFiles.size();) {
        if (!projectFiles.contains(openedFiles.at(i))) {
            openedFiles.removeAt(i);
        } else {
            projectFiles.removeOne(openedFiles.at(i));
            ++i;
        }
    }
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
    d->startAll();
}

void SearchReplaceWorker::addReplaceTask(const ReplaceParams &params)
{
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
}

void SearchReplaceWorker::handleReadSearchResult()
{
    const auto resultLineRegex = QRegularExpression(R"((.+):([0-9]+):(.+))", QRegularExpression::NoPatternOption);
    while (d->process->canReadLine()) {
        const auto &line = d->process->readLine();
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
