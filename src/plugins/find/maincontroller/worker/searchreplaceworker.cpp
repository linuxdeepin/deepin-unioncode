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

    process->setProgram(job.program);
    process->setArguments(job.arguments);
    process->start();
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
            jobList << job;
        }

        const auto &job = buildSearchJob(tmpParams.projectFileList, tmpParams.includeList, tmpParams.excludeList,
                                         tmpParams.keyword, tmpParams.flags, false);
        jobList << job;
    } break;
    case CurrentFile: {
        const auto &job = buildSearchJob(params.editFileList, params.includeList, params.excludeList,
                                         params.keyword, params.flags, true);
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
    job.program = "grep";
    job.keyword = keyword;
    job.flags = flags;
    job.arguments << "-Hn";
    if (!flags.testFlag(SearchCaseSensitively))
        job.arguments << "-i";

    if (flags.testFlag(SearchWholeWords)) {
        job.arguments << "-w";
    }

    if (flags.testFlag(SearchRegularExpression)) {
        job.arguments << "-P";
    } else {
        job.arguments << "-F";
    }

    if (!includeList.isEmpty())
        job.arguments << "--include=" + includeList.join(" --include=");
    if (!excludeList.isEmpty())
        job.arguments << "--exclude=" + excludeList.join(" --exclude=");
    job.arguments << keyword;

    if (isOpenedFile) {
        if (!editSrv)
            editSrv = dpfGetService(EditorService);
        job.channelData = editSrv->fileText(fileList.first());
        job.arguments << "--label=" + fileList.first();
    } else {
        job.arguments << fileList;
    }

    return job;
}

void SearchReplaceWorkerPrivate::parseResultWithRegExp(const QString &fileName, const QString &keyword,
                                                       const QString &contents, int line, SearchFlags flags)
{
    const QString term = flags & SearchWholeWords
            ? QString::fromLatin1("\\b%1\\b").arg(keyword)
            : keyword;
    const auto patternOptions = (flags & SearchCaseSensitively)
            ? QRegularExpression::NoPatternOption
            : QRegularExpression::CaseInsensitiveOption;
    const QRegularExpression expression = QRegularExpression(term, patternOptions);
    if (!expression.isValid())
        return;

    QRegularExpressionMatch match;
    int lengthOfContents = contents.length();
    int pos = 0;
    while ((match = expression.match(contents, pos)).hasMatch()) {
        pos = match.capturedStart();
        FindItem findItem;
        findItem.filePathName = fileName;
        findItem.line = line;
        findItem.keyword = keyword;
        findItem.context = contents;
        findItem.column = pos;
        findItem.matchedLength = match.capturedLength();
        findItem.capturedTexts = match.capturedTexts();

        {
            QMutexLocker lk(&mutex);
            searchResults.append(findItem);
        }
        ++resultCount;
        if (match.capturedLength() == 0)
            break;
        pos += match.capturedLength();
        if (pos >= lengthOfContents)
            break;
    }
}

void SearchReplaceWorkerPrivate::parseResultWithoutRegExp(const QString &fileName, const QString &keyword,
                                                          const QString &contents, int line, SearchFlags flags)
{
    const bool caseSensitive = (flags & SearchCaseSensitively);
    const bool wholeWord = (flags & SearchWholeWords);
    const QString keywordLower = keyword.toLower();
    const QString keywordUpper = keyword.toUpper();
    const int keywordMaxIndex = keyword.length() - 1;
    const QChar *keywordData = keyword.constData();
    const QChar *keywordDataLower = keywordLower.constData();
    const QChar *keywordDataUpper = keywordUpper.constData();

    const int contentsLength = contents.length();
    const QChar *contentsPtr = contents.constData();
    const QChar *contentsEnd = contentsPtr + contentsLength - 1;
    for (const QChar *regionPtr = contentsPtr; regionPtr + keywordMaxIndex <= contentsEnd; ++regionPtr) {
        const QChar *regionEnd = regionPtr + keywordMaxIndex;
        if ((caseSensitive && *regionPtr == keywordData[0]
             && *regionEnd == keywordData[keywordMaxIndex])
            ||
            // case insensitive
            (!caseSensitive && (*regionPtr == keywordDataLower[0] || *regionPtr == keywordDataUpper[0])
             && (*regionEnd == keywordDataLower[keywordMaxIndex]
                 || *regionEnd == keywordDataUpper[keywordMaxIndex]))) {
            bool equal = true;

            // whole word check
            const QChar *beforeRegion = regionPtr - 1;
            const QChar *afterRegion = regionEnd + 1;
            if (wholeWord
                && (((beforeRegion >= contentsPtr)
                     && (beforeRegion->isLetterOrNumber()
                         || ((*beforeRegion) == QLatin1Char('_'))))
                    || ((afterRegion <= contentsEnd)
                        && (afterRegion->isLetterOrNumber()
                            || ((*afterRegion) == QLatin1Char('_')))))) {
                equal = false;
            } else {
                // check all chars
                int regionIndex = 1;
                for (const QChar *regionCursor = regionPtr + 1;
                     regionCursor < regionEnd;
                     ++regionCursor, ++regionIndex) {
                    if (   // case sensitive
                            (caseSensitive
                             && *regionCursor != keywordData[regionIndex])
                            ||
                            // case insensitive
                            (!caseSensitive
                             && *regionCursor != keywordDataLower[regionIndex]
                             && *regionCursor != keywordDataUpper[regionIndex])) {
                        equal = false;
                        break;
                    }
                }
            }

            if (equal) {
                FindItem result;
                result.filePathName = fileName;
                result.line = line;
                result.column = regionPtr - contentsPtr;
                result.context = contents;
                result.keyword = keyword;
                result.matchedLength = keywordMaxIndex + 1;

                ++resultCount;
                regionPtr += keywordMaxIndex;

                QMutexLocker lk(&mutex);
                searchResults.append(result);
            }
        }
    }
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
        offset += newText.length() - item.matchedLength;
        lines[realLine].replace(index, item.matchedLength, newText);
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
        offset += newText.length() - item.matchedLength;
        editSrv->replaceRange(fileName, realLine, index, item.matchedLength, newText);
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

            flags.testFlag(SearchRegularExpression) ? d->parseResultWithRegExp(name, keyword, context, line, flags)
                                                    : d->parseResultWithoutRegExp(name, keyword, context, line, flags);
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
