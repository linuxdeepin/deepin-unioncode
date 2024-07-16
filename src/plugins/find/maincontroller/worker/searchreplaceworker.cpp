// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchreplaceworker.h"
#include "searchreplaceworker_p.h"

#include "common/util/qtcassert.h"

#include <QProcess>
#include <QMutex>
#include <QSharedPointer>
#include <QRegularExpression>

using namespace dpfservice;

inline constexpr int MaxResultCount { 20000 };

SearchReplaceWorkerPrivate::SearchReplaceWorkerPrivate(SearchReplaceWorker *qq)
    : q(qq)
{
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
    connect(process.get(), &QProcess::readyReadStandardOutput, q,
            std::bind(&SearchReplaceWorker::handleReadSearchResult, q, job.keyword, job.flags));
    connect(process.get(), qOverload<int>(&QProcess::finished), q, &SearchReplaceWorker::processDone);

    process->start(job.cmd);
    if (!job.channelData.isEmpty()) {
        process->write(job.channelData.toUtf8());
        process->closeWriteChannel();
    }
}

void SearchReplaceWorkerPrivate::createSearchJob(const SearchParams &params)
{
    jobList.clear();
    switch (params.scope) {
    case AllProjects:
    case CurrentProject: {
        auto tmpParams = params;
        processWorkingFiles(tmpParams.projectFileList, tmpParams.editFileList);
        for (const auto &file : tmpParams.editFileList) {
            const auto &job = buildSearchJob({ file }, tmpParams.includeList, tmpParams.excludeList,
                                             tmpParams.keyword, tmpParams.flags, true);
            if (!job.cmd.isEmpty())
                jobList << job;
        }

        const auto &job = buildSearchJob(tmpParams.projectFileList, tmpParams.includeList, tmpParams.excludeList,
                                         tmpParams.keyword, tmpParams.flags, false);
        if (!job.cmd.isEmpty())
            jobList << job;
    } break;
    case CurrentFile: {
        const auto &job = buildSearchJob(params.editFileList, params.includeList, params.excludeList,
                                         params.keyword, params.flags, true);
        if (!job.cmd.isEmpty())
            jobList << job;
    } break;
    default:
        break;
    }
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
    job.keyword = keyword;
    job.flags = flags;
    QStringList cmd;
    cmd << "grep -Hn";
    if (!flags.testFlag(SearchCaseSensitive))
        cmd << "-i";

    if (flags.testFlag(SearchWholeWord)) {
        cmd << "-w";
    }

    if (flags.testFlag(SearchRegularExpression)) {
        cmd << "-P";
    } else {
        cmd << "-F";
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

void SearchReplaceWorkerPrivate::replaceLocalFile(const QString &fileName, const QString &replacement, const FindItemList &itemList)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return;

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    file.seek(0);

    // When there are multiple results in a row,
    // the offset may change after replacement
    int offset = 0;
    int lastReplaceLine = 0;
    for (const auto &item : itemList) {
        int realLine = item.line - 1;
        if (lines.count() <= realLine)
            continue;

        QString newText = item.capturedTexts.isEmpty()
                ? replacement
                : Utils::expandRegExpReplacement(replacement, item.capturedTexts);
        int index = item.column;
        if (lastReplaceLine == realLine)
            index += offset;
        else
            offset = 0;

        lastReplaceLine = realLine;
        offset += newText.length() - item.matchedText.length();
        lines[realLine].replace(index, item.matchedText.length(), newText);
    }

    QTextStream out(&file);
    out << lines.join('\n');
    file.resize(file.pos());
    file.close();
}

void SearchReplaceWorkerPrivate::replaceOpenedFile(const QString &fileName, const QString &replacement, const FindItemList &itemList)
{
    if (!editSrv)
        editSrv = dpfGetService(EditorService);

    // When there are multiple results in a row,
    // the offset may change after replacement
    int offset = 0;
    int lastReplaceLine = 0;
    for (const auto &item : itemList) {
        int realLine = item.line - 1;
        QString newText = item.capturedTexts.isEmpty()
                ? replacement
                : Utils::expandRegExpReplacement(replacement, item.capturedTexts);
        int index = item.column;
        if (lastReplaceLine == realLine)
            index += offset;
        else
            offset = 0;

        lastReplaceLine = realLine;
        offset += newText.length() - item.matchedText.length();
        editSrv->replaceRange(fileName, realLine, index, item.matchedText.length(), newText);
    }
}

SearchReplaceWorker::SearchReplaceWorker(QObject *parent)
    : QObject(parent),
      d(new SearchReplaceWorkerPrivate(this))
{
    qRegisterMetaType<FindItemList>("FindItemList");
}

SearchReplaceWorker::~SearchReplaceWorker()
{
    stop();
    delete d;
}

void SearchReplaceWorker::stop()
{
    d->isStop = true;
    if (d->process) {
        disconnect(d->process.get(), nullptr, this, nullptr);
        d->process->kill();
        d->process.reset();
    }
}

bool SearchReplaceWorker::hasItem() const
{
    QMutexLocker lk(&d->mutex);
    return !d->searchResults.isEmpty();
}

FindItemList SearchReplaceWorker::takeAll()
{
    QMutexLocker lk(&d->mutex);
    return std::move(d->searchResults);
}

void SearchReplaceWorker::search(const SearchParams &params)
{
    d->resultCount = 0;
    d->createSearchJob(params);
    if (d->jobList.isEmpty()) {
        Q_EMIT finished();
        return;
    }

    d->startAll();
}

void SearchReplaceWorker::replace(const ReplaceParams &params)
{
    auto iter = params.resultMap.cbegin();
    for (; iter != params.resultMap.cend() && !d->isStop; ++iter) {
        if (params.editFileList.contains(iter.key())) {
            QMetaObject::invokeMethod(d, "replaceOpenedFile",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, iter.key()),
                                      Q_ARG(QString, params.replaceText),
                                      Q_ARG(FindItemList, iter.value()));
        } else {
            d->replaceLocalFile(iter.key(), params.replaceText, iter.value());
        }
    }

    Q_EMIT finished();
}

void SearchReplaceWorker::handleReadSearchResult(const QString &keyword, SearchFlags flags)
{
    const auto resultLineRegex = QRegularExpression(R"((.+):([0-9]+):(.+))", QRegularExpression::NoPatternOption);
    while (d->resultCount < MaxResultCount && d->process->canReadLine()) {
        const auto &line = d->process->readLine();
        QRegularExpressionMatch regMatch;
        if ((regMatch = resultLineRegex.match(line)).hasMatch()) {
            auto name = regMatch.captured(1);
            auto line = regMatch.captured(2).toInt();
            auto context = regMatch.captured(3);

            QString pattern = flags.testFlag(SearchWholeWord)
                    ? QString::fromLatin1("\\b%1\\b").arg(keyword)
                    : keyword;

            QRegularExpression regex(pattern, flags.testFlag(SearchCaseSensitive) ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatchIterator it = regex.globalMatch(context);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                if (match.hasMatch()) {
                    FindItem findItem;
                    findItem.filePathName = name;
                    findItem.line = line;
                    findItem.keyword = keyword;
                    findItem.context = context;
                    findItem.column = match.capturedStart();
                    findItem.matchedText = match.captured();
                    if (flags.testFlag(SearchRegularExpression))
                        findItem.capturedTexts = match.capturedTexts();

                    QMutexLocker lk(&d->mutex);
                    d->searchResults.append(findItem);
                    ++d->resultCount;
                }
            }
        }
    }

    if (hasItem())
        Q_EMIT matched();

    if (d->resultCount >= MaxResultCount) {
        stop();
        Q_EMIT finished();
    }
}

void SearchReplaceWorker::processDone()
{
    ++d->currentJob;
    if (d->currentJob < d->jobList.count()) {
        d->process.release()->deleteLater();
        d->startNextJob();
        return;
    }

    Q_EMIT finished();
}
