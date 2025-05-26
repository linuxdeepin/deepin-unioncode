// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diffutils.h"

#include <DGuiApplicationHelper>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QRegularExpression>

DGUI_USE_NAMESPACE

static int forBlockNumber(const QMap<int, QPair<int, int>> &chunkInfo, int blockNumber,
                          const std::function<int(int, int, int)> &func)
{
    if (chunkInfo.isEmpty())
        return -1;

    auto it = chunkInfo.upperBound(blockNumber);
    if (it == chunkInfo.constBegin())
        return -1;

    --it;

    if (blockNumber < it.key() + it.value().first)
        return func(it.key(), it.value().first, it.value().second);

    return -1;
}

static QList<DiffSelection> subtractSelection(
        const DiffSelection &minuendSelection,
        const DiffSelection &subtrahendSelection)
{
    // tha case that whole minuend is before the whole subtrahend
    if (minuendSelection.end >= 0 && minuendSelection.end <= subtrahendSelection.start)
        return { minuendSelection };

    // the case that whole subtrahend is before the whole minuend
    if (subtrahendSelection.end >= 0 && subtrahendSelection.end <= minuendSelection.start)
        return { minuendSelection };

    bool makeMinuendSubtrahendStart = false;
    bool makeSubtrahendMinuendEnd = false;

    if (minuendSelection.start < subtrahendSelection.start)
        makeMinuendSubtrahendStart = true;
    if (subtrahendSelection.end >= 0 && (subtrahendSelection.end < minuendSelection.end || minuendSelection.end < 0))
        makeSubtrahendMinuendEnd = true;

    QList<DiffSelection> diffList;
    if (makeMinuendSubtrahendStart)
        diffList += { minuendSelection.format, minuendSelection.start, subtrahendSelection.start };
    if (makeSubtrahendMinuendEnd)
        diffList += { minuendSelection.format, subtrahendSelection.end, minuendSelection.end };

    return diffList;
}

DiffSelections polishedSelections(const DiffSelections &selections)
{
    DiffSelections polishedSelections;
    for (auto it = selections.cbegin(), end = selections.cend(); it != end; ++it) {
        const QList<DiffSelection> diffSelections = it.value();
        QList<DiffSelection> workingList;
        for (const DiffSelection &diffSelection : diffSelections) {
            if (diffSelection.start == -1 && diffSelection.end == 0)
                continue;

            if (diffSelection.start == diffSelection.end && diffSelection.start >= 0)
                continue;

            int j = 0;
            while (j < workingList.count()) {
                const DiffSelection existingSelection = workingList.takeAt(j);
                const QList<DiffSelection> newSelection = subtractSelection(existingSelection, diffSelection);
                for (int k = 0; k < newSelection.count(); k++)
                    workingList.insert(j + k, newSelection.at(k));
                j += newSelection.count();
            }
            workingList.append(diffSelection);
        }
        polishedSelections.insert(it.key(), workingList);
    }
    return polishedSelections;
}

int DiffChunkInfo::chunkIndexForBlockNumber(int blockNumber) const
{
    return forBlockNumber(diffChunkInfo, blockNumber,
                          [](int, int, int chunkIndex) {
                              return chunkIndex;
                          });
}

int DiffChunkInfo::chunkRowForBlockNumber(int blockNumber) const
{
    return forBlockNumber(diffChunkInfo, blockNumber,
                          [blockNumber](int startBlockNumber, int, int) {
                              return blockNumber - startBlockNumber;
                          });
}

int DiffChunkInfo::chunkRowsCountForBlockNumber(int blockNumber) const
{
    return forBlockNumber(diffChunkInfo, blockNumber,
                          [](int, int rowsCount, int) {
                              return rowsCount;
                          });
}

DiffEditorInput::DiffEditorInput(const QList<FileData> &dataList, const QFont &font)
    : contextFileData(dataList)
{
    setFormat(font);
}

void DiffEditorInput::setFormat(const QFont &font)
{
    fileLineFormat.setFont(font);
    chunkLineFormat.setFont(font);
    spanLineFormat.setFont(font);
    lineFormat[LeftSide].setFont(font);
    lineFormat[RightSide].setFont(font);
    charFormat[LeftSide].setFont(font);
    charFormat[RightSide].setFont(font);

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        fileLineFormat.setBackground(QBrush("#ffff00"));
        chunkLineFormat.setBackground(QBrush("#afd7e7"));
        lineFormat[LeftSide].setBackground(QBrush("#ffdfdf"));
        lineFormat[RightSide].setBackground(QBrush("#dfffdf"));
        charFormat[LeftSide].setBackground(QBrush("#ffafaf"));
        charFormat[RightSide].setBackground(QBrush("#afffaf"));
    } else {
        fileLineFormat.setForeground(QBrush("#000000"));
        fileLineFormat.setBackground(QBrush("#ffff00"));

        chunkLineFormat.setForeground(QBrush("#000000"));
        chunkLineFormat.setBackground(QBrush("#8aaab6"));

        lineFormat[LeftSide].setBackground(QBrush("#8c2d2d"));
        lineFormat[RightSide].setBackground(QBrush("#2d8c2d"));

        charFormat[LeftSide].setForeground(QBrush("#000000"));
        charFormat[LeftSide].setBackground(QBrush("#c34141"));

        charFormat[RightSide].setForeground(QBrush("#000000"));
        charFormat[RightSide].setBackground(QBrush("#41c341"));
    }
}

GitDiffWorker::GitDiffWorker(QObject *parent)
    : QObject(parent)
{
}

void GitDiffWorker::stop()
{
    isStop = true;
}

QList<FileData> GitDiffWorker::getPatchResult() const
{
    return std::move(fileDataList);
}

SideBySideShowResults GitDiffWorker::getShowResults() const
{
    return std::move(showResults);
}

void GitDiffWorker::handleParsePatch(const QString &patch)
{
    isStop = false;
    fileDataList.clear();
    readPatch(fileDataList, patch);
}

void GitDiffWorker::handleParseFileDatas(const QList<FileData> &dataList, const QFont &font)
{
    isStop = false;
    const DiffEditorInput input(dataList, font);
    const SideBySideDiffOutput output = diffOutput(input);
    if (isStop) {
        Q_EMIT parseFileDatasFinished(false);
        return;
    }

    const SideBySideShowResult leftResult { QSharedPointer<QTextDocument>(new QTextDocument()),
                                            output.side[LeftSide].diffData, output.side[LeftSide].selections };
    const SideBySideShowResult rightResult { QSharedPointer<QTextDocument>(new QTextDocument()),
                                             output.side[RightSide].diffData, output.side[RightSide].selections };
    const SideBySideShowResults result { leftResult, rightResult };

    auto propagateDocument = [&output, this](DiffSide side, const SideBySideShowResult &result) {
        // No need to store the change history
        result.textDocument->setUndoRedoEnabled(false);

        // We could do just:
        //   result.textDocument->setPlainText(output.diffText);
        // but this would freeze the thread for couple of seconds without progress reporting
        // and without checking for canceled.
        const int diffSize = output.side[side].diffText.size();
        const int packageSize = 10000;
        int currentPos = 0;
        QTextCursor cursor(result.textDocument.data());
        while (currentPos < diffSize) {
            const QString package = output.side[side].diffText.mid(currentPos, packageSize);
            cursor.insertText(package);
            currentPos += package.size();
            if (isStop) {
                Q_EMIT parseFileDatasFinished(false);
                return;
            }
        }

        // If future was canceled, the destructor runs in this thread, so we can't move it
        // to caller's thread. We push it to no thread (make object to have no thread affinity),
        // and later, in the caller's thread, we pull it back to the caller's thread.
        result.textDocument->moveToThread(nullptr);
    };

    propagateDocument(LeftSide, leftResult);
    if (isStop) {
        Q_EMIT parseFileDatasFinished(false);
        return;
    }

    propagateDocument(RightSide, rightResult);
    if (isStop) {
        Q_EMIT parseFileDatasFinished(false);
        return;
    }

    showResults = result;
    Q_EMIT parseFileDatasFinished(true);
}

QList<ChunkData> GitDiffWorker::readChunks(const QString &patch, bool *lastChunkAtTheEndOfFile, bool *ok)
{
    QList<ChunkData> chunkDataList;
    int position = -1;

    QList<int> startingPositions;   // store starting positions of @@
    if (patch.startsWith(QStringLiteral("@@ -")))
        startingPositions.append(position + 1);

    while ((position = patch.indexOf(QStringLiteral("\n@@ -"), position + 1)) >= 0)
        startingPositions.append(position + 1);

    const QChar newLine('\n');
    bool readOk = true;
    const int count = startingPositions.size();
    for (int i = 0; i < count; i++) {
        const int chunkStart = startingPositions.at(i);
        const int chunkEnd = (i < count - 1)
                // drop the newline before the next chunk start
                ? startingPositions.at(i + 1) - 1
                // drop the possible newline by the end of patch
                : (patch.at(patch.size() - 1) == newLine ? patch.size() - 1
                                                         : patch.size());

        // extract just one chunk
        const QString chunkDiff = patch.mid(chunkStart, chunkEnd - chunkStart);
        ChunkData chunkData;
        QString lines;
        readOk = detectChunkData(chunkDiff, &chunkData, &lines);

        if (!readOk)
            break;

        chunkData.rows = readLines(lines, i == (startingPositions.size() - 1),
                                   lastChunkAtTheEndOfFile, &readOk);
        if (!readOk)
            break;

        chunkDataList.append(chunkData);
    }

    if (ok)
        *ok = readOk;

    return chunkDataList;
}

void GitDiffWorker::readGitPatch(QList<FileData> &dataList, const QString &patch)
{
    int position = -1;
    QList<int> startingPositions;   // store starting positions of git headers
    if (patch.startsWith(QStringLiteral("diff --git ")))
        startingPositions.append(position + 1);

    while ((position = patch.indexOf(QStringLiteral("\ndiff --git "), position + 1)) >= 0)
        startingPositions.append(position + 1);

    class PatchInfo
    {
    public:
        QString patch;
        FileData fileData;
    };

    const QChar newLine('\n');
    QList<PatchInfo> patches;
    const int count = startingPositions.size();
    for (int i = 0; i < count; i++) {
        if (isStop)
            return;

        const int diffStart = startingPositions.at(i);
        const int diffEnd = (i < count - 1)
                // drop the newline before the next header start
                ? startingPositions.at(i + 1) - 1
                // drop the possible newline by the end of file
                : (patch.at(patch.size() - 1) == newLine ? patch.size() - 1
                                                         : patch.size());

        // extract the patch for just one file
        const QString fileDiff = patch.mid(diffStart, diffEnd - diffStart);

        FileData fileData;
        QString remainingFileDiff;
        if (!detectFileData(fileDiff, &fileData, &remainingFileDiff))
            return;

        patches.append(PatchInfo { remainingFileDiff, fileData });
    }

    if (patches.isEmpty())
        return;

    for (const auto &patchInfo : std::as_const(patches)) {
        if (isStop)
            return;

        FileData fileData = patchInfo.fileData;
        bool readOk = false;
        if (!patchInfo.patch.isEmpty() || fileData.fileOperation == FileData::ChangeFile)
            fileData.chunks = readChunks(patchInfo.patch, &fileData.lastChunkAtTheEndOfFile, &readOk);
        else
            readOk = true;

        if (!readOk)
            return;

        dataList.append(fileData);
    }
}

void GitDiffWorker::readPatch(QList<FileData> &dataList, const QString &patch)
{
    QString tmpPath = patch;
    const QRegularExpression formatPatchEndingRegExp("(\\n-- \\n\\S*\\n\\n$)");
    auto match = formatPatchEndingRegExp.match(tmpPath);
    if (match.hasMatch())
        tmpPath = tmpPath.left(match.capturedStart() + 1);

    readGitPatch(dataList, tmpPath);
    if (dataList.isEmpty())
        readDiffPatch(dataList, tmpPath);

    Q_EMIT parsePatchFinished();
}

void GitDiffWorker::readDiffPatch(QList<FileData> &dataList, const QString &patch)
{
    const QRegularExpression diffRegExp("(?:\\n|^)"   // new line of the beginning of a patch
                                        "("   // either
                                        "-{3} "   // ---
                                        "[^\\t\\n]+"   // filename1
                                        "(?:\\t[^\\n]*)*\\n"   // optionally followed by: \t anything \t anything ...
                                        "\\+{3} "   // +++
                                        "[^\\t\\n]+"   // filename2
                                        "(?:\\t[^\\n]*)*\\n"   // optionally followed by: \t anything \t anything ...
                                        "|"   // or
                                        "Binary files "
                                        "[^\\t\\n]+"   // filename1
                                        " and "
                                        "[^\\t\\n]+"   // filename2
                                        " differ"
                                        ")");   // end of or

    bool readOk = false;
    QRegularExpressionMatch diffMatch = diffRegExp.match(patch);
    if (diffMatch.hasMatch()) {
        readOk = true;
        int lastPos = -1;
        do {
            if (isStop)
                return;

            int pos = diffMatch.capturedStart();
            if (lastPos >= 0) {
                QString headerAndChunks = patch.mid(lastPos, pos - lastPos);
                const FileData fileData = readDiffHeaderAndChunks(headerAndChunks, &readOk);
                if (!readOk)
                    break;

                dataList.append(fileData);
            }
            lastPos = pos;
            pos = diffMatch.capturedEnd();
            diffMatch = diffRegExp.match(patch, pos);
        } while (diffMatch.hasMatch());

        if (readOk) {
            QString headerAndChunks = patch.mid(lastPos, patch.size() - lastPos - 1);
            const FileData fileData = readDiffHeaderAndChunks(headerAndChunks, &readOk);
            if (readOk)
                dataList.append(fileData);
        }
    }
    if (!readOk)
        return;
}

FileData GitDiffWorker::readDiffHeaderAndChunks(const QString &headerAndChunks, bool *ok)
{
    QString patch = headerAndChunks;
    FileData fileData;
    bool readOk = false;

    const QRegularExpression leftFileRegExp(
            "(?:\\n|^)-{3} "   // "--- "
            "([^\\t\\n]+)"   // "fileName1"
            "(?:\\t[^\\n]*)*\\n");   // optionally followed by: \t anything \t anything ...)
    const QRegularExpression rightFileRegExp(
            "^\\+{3} "   // "+++ "
            "([^\\t\\n]+)"   // "fileName2"
            "(?:\\t[^\\n]*)*\\n");   // optionally followed by: \t anything \t anything ...)
    const QRegularExpression binaryRegExp(
            "^Binary files ([^\\t\\n]+) and ([^\\t\\n]+) differ$");

    // followed either by leftFileRegExp
    const QRegularExpressionMatch leftMatch = leftFileRegExp.match(patch);
    if (leftMatch.hasMatch() && leftMatch.capturedStart() == 0) {
        patch = patch.mid(leftMatch.capturedEnd());
        fileData.fileInfo[LeftSide].fileName = leftMatch.captured(1);

        // followed by rightFileRegExp
        const QRegularExpressionMatch rightMatch = rightFileRegExp.match(patch);
        if (rightMatch.hasMatch() && rightMatch.capturedStart() == 0) {
            patch = patch.mid(rightMatch.capturedEnd());
            fileData.fileInfo[RightSide].fileName = rightMatch.captured(1);

            fileData.chunks = readChunks(patch,
                                         &fileData.lastChunkAtTheEndOfFile,
                                         &readOk);
        }
    } else {
        // or by binaryRegExp
        const QRegularExpressionMatch binaryMatch = binaryRegExp.match(patch);
        if (binaryMatch.hasMatch() && binaryMatch.capturedStart() == 0) {
            fileData.fileInfo[LeftSide].fileName = binaryMatch.captured(1);
            fileData.fileInfo[RightSide].fileName = binaryMatch.captured(2);
            fileData.binaryFiles = true;
            readOk = true;
        }
    }

    if (ok)
        *ok = readOk;

    if (!readOk)
        return {};

    return fileData;
}

ChunkData GitDiffWorker::calculateOriginalData(const QList<Diff> &leftDiffList,
                                               const QList<Diff> &rightDiffList)
{
    int i = 0;
    int j = 0;

    QList<TextLineData> leftLines;
    QList<TextLineData> rightLines;

    // <line number, span count>
    QMap<int, int> leftSpans;
    QMap<int, int> rightSpans;
    // <left line number, right line number>
    QMap<int, int> equalLines;

    int leftLineNumber = 0;
    int rightLineNumber = 0;
    int leftLineAligned = -1;
    int rightLineAligned = -1;
    bool lastLineEqual = true;

    while (i <= leftDiffList.size() && j <= rightDiffList.size()) {
        const Diff leftDiff = i < leftDiffList.size() ? leftDiffList.at(i) : Diff(Diff::Equal);
        const Diff rightDiff = j < rightDiffList.size() ? rightDiffList.at(j) : Diff(Diff::Equal);

        if (leftDiff.command == Diff::Delete) {
            if (j == rightDiffList.size() && lastLineEqual && leftDiff.text.startsWith('\n'))
                equalLines.insert(leftLineNumber, rightLineNumber);
            // process delete
            handleDifference(leftDiff.text, &leftLines, &leftLineNumber);
            lastLineEqual = lastLinesEqual(leftLines, rightLines);
            if (j == rightDiffList.size())
                lastLineEqual = false;
            i++;
        }
        if (rightDiff.command == Diff::Insert) {
            if (i == leftDiffList.size() && lastLineEqual && rightDiff.text.startsWith('\n'))
                equalLines.insert(leftLineNumber, rightLineNumber);
            // process insert
            handleDifference(rightDiff.text, &rightLines, &rightLineNumber);
            lastLineEqual = lastLinesEqual(leftLines, rightLines);
            if (i == leftDiffList.size())
                lastLineEqual = false;
            j++;
        }
        if (leftDiff.command == Diff::Equal && rightDiff.command == Diff::Equal) {
            // process equal
            const QStringList newLeftLines = leftDiff.text.split('\n');
            const QStringList newRightLines = rightDiff.text.split('\n');

            int line = 0;

            if (i < leftDiffList.size() || j < rightDiffList.size()
                || (!leftLines.isEmpty() && !rightLines.isEmpty())) {
                while (line < qMax(newLeftLines.size(), newRightLines.size())) {
                    handleLine(newLeftLines, line, &leftLines, &leftLineNumber);
                    handleLine(newRightLines, line, &rightLines, &rightLineNumber);

                    const int commonLineCount = qMin(newLeftLines.size(), newRightLines.size());
                    if (line < commonLineCount) {
                        // try to align
                        const int leftDifference = leftLineNumber - leftLineAligned;
                        const int rightDifference = rightLineNumber - rightLineAligned;

                        if (leftDifference && rightDifference) {
                            bool doAlign = true;
                            if (line == 0
                                && (newLeftLines.at(0).isEmpty()
                                    || newRightLines.at(0).isEmpty())
                                && !lastLineEqual) {
                                // omit alignment when first lines of equalities
                                // are empty and last generated lines are not equal
                                doAlign = false;
                            }

                            if (line == commonLineCount - 1) {
                                // omit alignment when last lines of equalities are empty
                                if (leftLines.last().text.isEmpty()
                                    || rightLines.last().text.isEmpty())
                                    doAlign = false;

                                // unless it's the last dummy line (don't omit in that case)
                                if (i == leftDiffList.size() && j == rightDiffList.size())
                                    doAlign = true;
                            }

                            if (doAlign) {
                                // align here
                                leftLineAligned = leftLineNumber;
                                rightLineAligned = rightLineNumber;

                                // insert separators if needed
                                if (rightDifference > leftDifference)
                                    leftSpans.insert(leftLineNumber,
                                                     rightDifference - leftDifference);
                                else if (leftDifference > rightDifference)
                                    rightSpans.insert(rightLineNumber,
                                                      leftDifference - rightDifference);
                            }
                        }
                    }

                    // check if lines are equal
                    if ((line < commonLineCount - 1)   // before the last common line in equality
                        || (line == commonLineCount - 1   // or the last common line in equality
                            && i == leftDiffList.size()   // and it's the last iteration
                            && j == rightDiffList.size())) {
                        if (line > 0 || lastLineEqual)
                            equalLines.insert(leftLineNumber, rightLineNumber);
                    }

                    if (line > 0)
                        lastLineEqual = true;

                    line++;
                }
            }
            i++;
            j++;
        }
    }

    QList<TextLineData> leftData = assemblyRows(leftLines, leftSpans);
    QList<TextLineData> rightData = assemblyRows(rightLines, rightSpans);

    // fill ending separators
    for (int i = leftData.size(); i < rightData.size(); i++)
        leftData.append(TextLineData(TextLineData::Separator));
    for (int i = rightData.size(); i < leftData.size(); i++)
        rightData.append(TextLineData(TextLineData::Separator));

    const int visualLineCount = leftData.size();
    int leftLine = -1;
    int rightLine = -1;
    ChunkData chunkData;

    for (int i = 0; i < visualLineCount; i++) {
        const TextLineData &leftTextLine = leftData.at(i);
        const TextLineData &rightTextLine = rightData.at(i);
        RowData row(leftTextLine, rightTextLine);

        if (leftTextLine.textLineType == TextLineData::TextLine)
            ++leftLine;
        if (rightTextLine.textLineType == TextLineData::TextLine)
            ++rightLine;
        if (equalLines.value(leftLine, -2) == rightLine)
            row.equal = true;

        chunkData.rows.append(row);
    }
    return chunkData;
}

bool GitDiffWorker::detectChunkData(const QString &chunkDiff, ChunkData *chunkData, QString *remainingPatch)
{
    bool hasNewLine;
    const QString chunkLine = readLine(chunkDiff, remainingPatch, &hasNewLine);

    const QLatin1String leftPosMarker("@@ -");
    const QLatin1String rightPosMarker(" +");
    const QLatin1String optionalHintMarker(" @@");

    const int leftPosIndex = chunkLine.indexOf(leftPosMarker);
    if (leftPosIndex != 0)
        return false;

    const int rightPosIndex = chunkLine.indexOf(rightPosMarker, leftPosIndex + leftPosMarker.size());
    if (rightPosIndex < 0)
        return false;

    const int optionalHintIndex = chunkLine.indexOf(optionalHintMarker, rightPosIndex + rightPosMarker.size());
    if (optionalHintIndex < 0)
        return false;

    const int leftPosStart = leftPosIndex + leftPosMarker.size();
    const int leftPosLength = rightPosIndex - leftPosStart;
    QString leftPos = chunkLine.mid(leftPosStart, leftPosLength);

    const int rightPosStart = rightPosIndex + rightPosMarker.size();
    const int rightPosLength = optionalHintIndex - rightPosStart;
    QString rightPos = chunkLine.mid(rightPosStart, rightPosLength);

    const int optionalHintStart = optionalHintIndex + optionalHintMarker.size();
    const int optionalHintLength = chunkLine.size() - optionalHintStart;
    const QString optionalHint = chunkLine.mid(optionalHintStart, optionalHintLength);

    const QChar comma(',');
    bool ok;

    const int leftCommaIndex = leftPos.indexOf(comma);
    if (leftCommaIndex >= 0)
        leftPos = leftPos.left(leftCommaIndex);
    const int leftLineNumber = leftPos.toInt(&ok);
    if (!ok)
        return false;

    const int rightCommaIndex = rightPos.indexOf(comma);
    if (rightCommaIndex >= 0)
        rightPos = rightPos.left(rightCommaIndex);
    const int rightLineNumber = rightPos.toInt(&ok);
    if (!ok)
        return false;

    chunkData->startingLineNumber = { leftLineNumber - 1, rightLineNumber - 1 };
    chunkData->contextInfo = optionalHint;

    return true;
}

QString GitDiffWorker::readLine(const QString &text, QString *remainingText, bool *hasNewLine)
{
    const QChar newLine('\n');
    const int indexOfFirstNewLine = text.indexOf(newLine);
    if (indexOfFirstNewLine < 0) {
        if (remainingText)
            *remainingText = QString();
        if (hasNewLine)
            *hasNewLine = false;
        return text;
    }

    if (hasNewLine)
        *hasNewLine = true;

    if (remainingText)
        *remainingText = text.mid(indexOfFirstNewLine + 1);

    return text.left(indexOfFirstNewLine);
}

QList<RowData> GitDiffWorker::readLines(const QString &patch, bool lastChunk, bool *lastChunkAtTheEndOfFile, bool *ok)
{
    QList<Diff> diffList;
    const QChar newLine = '\n';
    int lastEqual = -1;
    int lastDelete = -1;
    int lastInsert = -1;
    int noNewLineInEqual = -1;
    int noNewLineInDelete = -1;
    int noNewLineInInsert = -1;

    const QList<QString> lines = patch.split(newLine);
    int i;
    for (i = 0; i < lines.size(); i++) {
        QString line = lines.at(i);
        if (line.isEmpty()) {   // need to have at least one character (1 column)
            if (lastChunk)
                i = lines.size();   // pretend as we've read all the lines (we just ignore the rest)
            break;
        }
        const QChar firstCharacter = line.at(0);
        if (firstCharacter == '\\') {   // no new line marker
            if (!lastChunk)   // can only appear in last chunk of the file
                break;
            if (!diffList.isEmpty()) {
                Diff &last = diffList.last();
                if (last.text.isEmpty())
                    break;

                if (last.command == Diff::Equal) {
                    if (noNewLineInEqual >= 0)
                        break;
                    noNewLineInEqual = diffList.size() - 1;
                } else if (last.command == Diff::Delete) {
                    if (noNewLineInDelete >= 0)
                        break;
                    noNewLineInDelete = diffList.size() - 1;
                } else if (last.command == Diff::Insert) {
                    if (noNewLineInInsert >= 0)
                        break;
                    noNewLineInInsert = diffList.size() - 1;
                }
            }
        } else {
            Diff::Command command = Diff::Equal;
            if (firstCharacter == ' ') {   // common line
                command = Diff::Equal;
            } else if (firstCharacter == '-') {   // deleted line
                command = Diff::Delete;
            } else if (firstCharacter == '+') {   // inserted line
                command = Diff::Insert;
            } else {   // no other character may exist as the first character
                if (lastChunk)
                    i = lines.size();   // pretend as we've read all the lines (we just ignore the rest)
                break;
            }

            Diff diffToBeAdded(command, line.mid(1) + newLine);
            if (!diffList.isEmpty() && diffList.last().command == command)
                diffList.last().text.append(diffToBeAdded.text);
            else
                diffList.append(diffToBeAdded);

            if (command == Diff::Equal)   // common line
                lastEqual = diffList.size() - 1;
            else if (command == Diff::Delete)   // deleted line
                lastDelete = diffList.size() - 1;
            else if (command == Diff::Insert)   // inserted line
                lastInsert = diffList.size() - 1;
        }
    }

    if (i < lines.size()   // we broke before
        // or we have noNewLine in some equal line and in either delete or insert line
        || (noNewLineInEqual >= 0 && (noNewLineInDelete >= 0 || noNewLineInInsert >= 0))
        // or we have noNewLine in not the last equal line
        || (noNewLineInEqual >= 0 && noNewLineInEqual != lastEqual)
        // or we have noNewLine in not the last delete line or there is a equal line after the noNewLine for delete
        || (noNewLineInDelete >= 0 && (noNewLineInDelete != lastDelete || lastEqual > lastDelete))
        // or we have noNewLine in not the last insert line or there is a equal line after the noNewLine for insert
        || (noNewLineInInsert >= 0 && (noNewLineInInsert != lastInsert || lastEqual > lastInsert))) {
        if (ok)
            *ok = false;
        return {};
    }

    if (ok)
        *ok = true;

    bool removeNewLineFromLastEqual = false;
    bool removeNewLineFromLastDelete = false;
    bool removeNewLineFromLastInsert = false;
    bool prependNewLineAfterLastEqual = false;

    if (noNewLineInDelete >= 0 || noNewLineInInsert >= 0) {
        if (noNewLineInDelete >= 0)
            removeNewLineFromLastDelete = true;
        if (noNewLineInInsert >= 0)
            removeNewLineFromLastInsert = true;
    } else {
        if (noNewLineInEqual >= 0) {
            removeNewLineFromLastEqual = true;
        } else {
            if (lastEqual > lastDelete && lastEqual > lastInsert) {
                removeNewLineFromLastEqual = true;
            } else if (lastDelete > lastEqual && lastDelete > lastInsert) {
                if (lastInsert > lastEqual) {
                    removeNewLineFromLastDelete = true;
                    removeNewLineFromLastInsert = true;
                } else if (lastEqual > lastInsert) {
                    removeNewLineFromLastEqual = true;
                    removeNewLineFromLastDelete = true;
                    prependNewLineAfterLastEqual = true;
                }
            } else if (lastInsert > lastEqual && lastInsert > lastDelete) {
                if (lastDelete > lastEqual) {
                    removeNewLineFromLastDelete = true;
                    removeNewLineFromLastInsert = true;
                } else if (lastEqual > lastDelete) {
                    removeNewLineFromLastEqual = true;
                    removeNewLineFromLastInsert = true;
                    prependNewLineAfterLastEqual = true;
                }
            }
        }
    }

    if (removeNewLineFromLastEqual) {
        Diff &diff = diffList[lastEqual];
        diff.text = diff.text.left(diff.text.size() - 1);
    }
    if (removeNewLineFromLastDelete) {
        Diff &diff = diffList[lastDelete];
        diff.text = diff.text.left(diff.text.size() - 1);
    }
    if (removeNewLineFromLastInsert) {
        Diff &diff = diffList[lastInsert];
        diff.text = diff.text.left(diff.text.size() - 1);
    }
    if (prependNewLineAfterLastEqual) {
        Diff &diff = diffList[lastEqual + 1];
        diff.text = newLine + diff.text;
    }

    if (lastChunkAtTheEndOfFile) {
        *lastChunkAtTheEndOfFile = noNewLineInEqual >= 0
                || noNewLineInDelete >= 0 || noNewLineInInsert >= 0;
    }

    //    diffList = Differ::merge(diffList);
    QList<Diff> leftDiffList;
    QList<Diff> rightDiffList;
    Differ::splitDiffList(diffList, &leftDiffList, &rightDiffList);
    QList<Diff> outputLeftDiffList;
    QList<Diff> outputRightDiffList;

    Differ::diffBetweenEqualities(leftDiffList,
                                  rightDiffList,
                                  &outputLeftDiffList,
                                  &outputRightDiffList);

    return calculateOriginalData(outputLeftDiffList,
                                 outputRightDiffList)
            .rows;
}

bool GitDiffWorker::detectFileData(const QString &patch, FileData *fileData, QString *remainingPatch)
{
    bool hasNewLine;
    QString afterDiffGit;
    // diff --git a/leftFileName b/rightFileName
    const QString diffGit = readLine(patch, &afterDiffGit, &hasNewLine);
    if (!hasNewLine)
        return false;   // we need to have at least one more line

    const QLatin1String gitHeader("diff --git ");
    const QString fileNames = diffGit.mid(gitHeader.size());
    QString commonFileName;
    if (extractCommonFileName(fileNames, &commonFileName)) {
        // change / new / delete

        fileData->fileOperation = FileData::ChangeFile;
        fileData->fileInfo[LeftSide].fileName = fileData->fileInfo[RightSide].fileName = commonFileName;

        QString afterSecondLine;
        const QString secondLine = readLine(afterDiffGit, &afterSecondLine, &hasNewLine);

        if (secondLine.startsWith(QStringLiteral("new file mode "))) {
            fileData->fileOperation = FileData::NewFile;
            *remainingPatch = afterSecondLine;
        } else if (secondLine.startsWith(QStringLiteral("deleted file mode "))) {
            fileData->fileOperation = FileData::DeleteFile;
            *remainingPatch = afterSecondLine;
        } else if (secondLine.startsWith(QStringLiteral("old mode "))) {
            QString afterThirdLine;
            // new mode
            readLine(afterSecondLine, &afterThirdLine, &hasNewLine);
            if (!hasNewLine)
                fileData->fileOperation = FileData::ChangeMode;

            // TODO: validate new mode line
            *remainingPatch = afterThirdLine;
        } else {
            *remainingPatch = afterDiffGit;
        }

    } else {
        // copy / rename
        QString afterModeOrSimilarity;
        QString afterSimilarity;
        const QString secondLine = readLine(afterDiffGit, &afterModeOrSimilarity, &hasNewLine);
        if (secondLine.startsWith(QLatin1String("old mode "))) {
            if (!hasNewLine)
                return false;
            readLine(afterModeOrSimilarity, &afterModeOrSimilarity, &hasNewLine);   // new mode
            if (!hasNewLine)
                return false;
            // (dis)similarity index [0-100]%
            readLine(afterModeOrSimilarity, &afterSimilarity, &hasNewLine);
        } else {
            afterSimilarity = afterModeOrSimilarity;
        }

        if (!hasNewLine)
            return false;   // we need to have at least one more line

        QString afterCopyRenameFrom;
        // [copy / rename] from leftFileName
        const QString copyRenameFrom = readLine(afterSimilarity, &afterCopyRenameFrom, &hasNewLine);
        if (!hasNewLine)
            return false;   // we need to have at least one more line

        const QLatin1String copyFrom("copy from ");
        const QLatin1String renameFrom("rename from ");
        if (copyRenameFrom.startsWith(copyFrom)) {
            fileData->fileOperation = FileData::CopyFile;
            fileData->fileInfo[LeftSide].fileName = copyRenameFrom.mid(copyFrom.size());
        } else if (copyRenameFrom.startsWith(renameFrom)) {
            fileData->fileOperation = FileData::RenameFile;
            fileData->fileInfo[LeftSide].fileName = copyRenameFrom.mid(renameFrom.size());
        } else {
            return false;
        }

        QString afterCopyRenameTo;
        // [copy / rename] to rightFileName
        const QString copyRenameTo = readLine(afterCopyRenameFrom, &afterCopyRenameTo, &hasNewLine);

        // if (dis)similarity index is 100% we don't have more lines

        const QLatin1String copyTo("copy to ");
        const QLatin1String renameTo("rename to ");
        if (fileData->fileOperation == FileData::CopyFile && copyRenameTo.startsWith(copyTo)) {
            fileData->fileInfo[RightSide].fileName = copyRenameTo.mid(copyTo.size());
        } else if (fileData->fileOperation == FileData::RenameFile && copyRenameTo.startsWith(renameTo)) {
            fileData->fileInfo[RightSide].fileName = copyRenameTo.mid(renameTo.size());
        } else {
            return false;
        }

        *remainingPatch = afterCopyRenameTo;
    }
    return detectIndexAndBinary(*remainingPatch, fileData, remainingPatch);
}

bool GitDiffWorker::extractCommonFileName(const QString &fileNames, QString *fileName)
{
    // we should have 1 space between filenames
    if (fileNames.size() % 2 == 0)
        return false;

    if (!fileNames.startsWith(QLatin1String("a/")))
        return false;

    // drop the space in between
    const int fileNameSize = fileNames.size() / 2;
    if (!fileNames.mid(fileNameSize).startsWith(QLatin1String(" b/")))
        return false;

    // drop "a/"
    const QString leftFileName = fileNames.mid(2, fileNameSize - 2);

    // drop the first filename + " b/"
    const QString rightFileName = fileNames.mid(fileNameSize + 3, fileNameSize - 2);

    if (leftFileName != rightFileName)
        return false;

    *fileName = leftFileName;
    return true;
}

bool GitDiffWorker::detectIndexAndBinary(const QString &patch, FileData *fileData, QString *remainingPatch)
{
    bool hasNewLine;
    *remainingPatch = patch;

    if (remainingPatch->isEmpty()) {
        switch (fileData->fileOperation) {
        case FileData::CopyFile:
        case FileData::RenameFile:
        case FileData::ChangeMode:
            // in case of 100% similarity we don't have more lines in the patch
            return true;
        default:
            break;
        }
    }

    QString afterNextLine;
    // index [leftIndexSha]..[rightIndexSha] <optionally: octalNumber>
    const QString nextLine = readLine(patch, &afterNextLine, &hasNewLine);

    const QLatin1String indexHeader("index ");

    if (nextLine.startsWith(indexHeader)) {
        const QString indices = nextLine.mid(indexHeader.size());
        const int dotsPosition = indices.indexOf(QStringLiteral(".."));
        if (dotsPosition < 0)
            return false;
        fileData->fileInfo[LeftSide].typeInfo = indices.left(dotsPosition);

        // if there is no space we take the remaining string
        const int spacePosition = indices.indexOf(QChar::Space, dotsPosition + 2);
        const int length = spacePosition < 0 ? -1 : spacePosition - dotsPosition - 2;
        fileData->fileInfo[RightSide].typeInfo = indices.mid(dotsPosition + 2, length);

        *remainingPatch = afterNextLine;
    } else if (fileData->fileOperation != FileData::ChangeFile) {
        // no index only in case of ChangeFile,
        // the dirty submodule diff case, see "Dirty Submodule" test:
        return false;
    }

    if (remainingPatch->isEmpty() && (fileData->fileOperation == FileData::NewFile || fileData->fileOperation == FileData::DeleteFile)) {
        // OK in case of empty file
        return true;
    }

    const QString leftFileName = sideFileName(LeftSide, *fileData);
    const QString rightFileName = sideFileName(RightSide, *fileData);
    const QString binaryLine = "Binary files "
            + leftFileName + " and "
            + rightFileName + " differ";

    if (*remainingPatch == binaryLine) {
        fileData->binaryFiles = true;
        *remainingPatch = QString();
        return true;
    }

    const QString leftStart = "--- " + leftFileName;
    QString afterMinuses;
    // --- leftFileName
    const QString minuses = readLine(*remainingPatch, &afterMinuses, &hasNewLine);
    if (!hasNewLine)
        return false;   // we need to have at least one more line

    if (!minuses.startsWith(leftStart))
        return false;

    const QString rightStart = "+++ " + rightFileName;
    QString afterPluses;
    // +++ rightFileName
    const QString pluses = readLine(afterMinuses, &afterPluses, &hasNewLine);
    if (!hasNewLine)
        return false;   // we need to have at least one more line

    if (!pluses.startsWith(rightStart))
        return false;

    *remainingPatch = afterPluses;
    return true;
}

QString GitDiffWorker::sideFileName(DiffSide side, const FileData &fileData)
{
    const FileData::FileOperation operation = side == LeftSide ? FileData::NewFile
                                                               : FileData::DeleteFile;
    if (fileData.fileOperation == operation)
        return "/dev/null";
    const QString sideMarker = side == LeftSide ? QString("a/") : QString("b/");
    return sideMarker + fileData.fileInfo[side].fileName;
}

void GitDiffWorker::handleDifference(const QString &text, QList<TextLineData> *lines, int *lineNumber)
{
    const QStringList newLines = text.split('\n');
    for (int line = 0; line < newLines.size(); ++line) {
        const int startPos = line > 0 ? -1 : lines->isEmpty() ? 0 : lines->last().text.size();
        handleLine(newLines, line, lines, lineNumber);
        const int endPos = line < newLines.size() - 1
                ? -1
                : lines->isEmpty() ? 0 : lines->last().text.size();
        if (!lines->isEmpty())
            lines->last().changedPositions.insert(startPos, endPos);
    }
}

void GitDiffWorker::handleLine(const QStringList &newLines, int line, QList<TextLineData> *lines, int *lineNumber)
{
    if (line < newLines.size()) {
        const QString text = newLines.at(line);
        if (lines->isEmpty() || line > 0) {
            if (line > 0)
                ++*lineNumber;
            lines->append(TextLineData(text));
        } else {
            lines->last().text += text;
        }
    }
}

bool GitDiffWorker::lastLinesEqual(const QList<TextLineData> &leftLines, const QList<TextLineData> &rightLines)
{
    const bool leftLineEqual = leftLines.isEmpty() || leftLines.last().text.isEmpty();
    const bool rightLineEqual = rightLines.isEmpty() || rightLines.last().text.isEmpty();
    return leftLineEqual && rightLineEqual;
}

QList<TextLineData> GitDiffWorker::assemblyRows(const QList<TextLineData> &lines, const QMap<int, int> &lineSpans)
{
    QList<TextLineData> data;

    const int lineCount = lines.size();
    for (int i = 0; i <= lineCount; i++) {
        for (int j = 0; j < lineSpans.value(i); j++)
            data.append(TextLineData(TextLineData::Separator));
        if (i < lineCount)
            data.append(lines.at(i));
    }
    return data;
}

SideBySideDiffOutput GitDiffWorker::diffOutput(const DiffEditorInput &input)
{
    SideBySideDiffOutput output;

    const QChar separator = '\n';
    int blockNumber = 0;
    std::array<QString, SideCount> diffText {};

    auto addFileLine = [&](DiffSide side, const FileData &fileData) {
        output.side[side].selections[blockNumber].append({ input.fileLineFormat });
        output.side[side].diffData.setFileInfo(blockNumber, fileData.fileInfo[side]);
        diffText[side] += separator;
    };

    auto addChunkLine = [&](DiffSide side, int skippedLines, const QString &contextInfo = {}) {
        output.side[side].selections[blockNumber].append({ input.chunkLineFormat });
        output.side[side].diffData.setSkippedLines(blockNumber, skippedLines, contextInfo);
        diffText[side] += separator;
    };

    auto addRowLine = [&](DiffSide side, const RowData &rowData,
                          int *lineNumber, int *lastLineNumber = nullptr) {
        if (rowData.line[side].textLineType == TextLineData::TextLine) {
            diffText[side] += rowData.line[side].text;
            if (lastLineNumber)
                *lastLineNumber = *lineNumber;
            ++(*lineNumber);
            output.side[side].diffData.setLineNumber(blockNumber, *lineNumber);
        } else if (rowData.line[side].textLineType == TextLineData::Separator) {
            output.side[side].diffData.setSeparator(blockNumber, true);
        }

        if (!rowData.equal) {
            if (rowData.line[side].textLineType == TextLineData::TextLine)
                output.side[side].selections[blockNumber].append({ input.lineFormat[side] });
            else
                output.side[side].selections[blockNumber].append({ input.spanLineFormat });
        }
        for (auto it = rowData.line[side].changedPositions.cbegin(),
                  end = rowData.line[side].changedPositions.cend();
             it != end; ++it) {
            output.side[side].selections[blockNumber].append(
                    { input.charFormat[side], it.key(), it.value() });
        }
        diffText[side] += separator;
    };

    auto addSkippedLine = [&](DiffSide side, int skippedLines) {
        output.side[side].selections[blockNumber].append({ input.chunkLineFormat });
        output.side[side].diffData.setSkippedLines(blockNumber, skippedLines);
        diffText[side] += separator;
    };

    for (const FileData &contextFileData : input.contextFileData) {
        diffText = {};
        output.foldingIndent.insert(blockNumber, 1);

        addFileLine(LeftSide, contextFileData);
        addFileLine(RightSide, contextFileData);
        blockNumber++;

        int lastLeftLineNumber = -1;

        if (contextFileData.binaryFiles) {
            output.foldingIndent.insert(blockNumber, 2);
            addChunkLine(LeftSide, -2);
            addChunkLine(RightSide, -2);
            blockNumber++;
        } else {
            for (int j = 0; j < contextFileData.chunks.size(); j++) {
                const ChunkData &chunkData = contextFileData.chunks.at(j);

                int leftLineNumber = chunkData.startingLineNumber[LeftSide];
                int rightLineNumber = chunkData.startingLineNumber[RightSide];

                if (!chunkData.contextChunk) {
                    const int skippedLines = leftLineNumber - lastLeftLineNumber - 1;
                    if (skippedLines > 0) {
                        output.foldingIndent.insert(blockNumber, 2);
                        addChunkLine(LeftSide, skippedLines, chunkData.contextInfo);
                        addChunkLine(RightSide, skippedLines, chunkData.contextInfo);
                        blockNumber++;
                    }

                    const int rows = chunkData.rows.size();
                    output.side[LeftSide].diffData.diffChunkInfo.setChunkIndex(blockNumber, rows, j);
                    output.side[RightSide].diffData.diffChunkInfo.setChunkIndex(blockNumber, rows, j);

                    for (const RowData &rowData : chunkData.rows) {
                        addRowLine(LeftSide, rowData, &leftLineNumber, &lastLeftLineNumber);
                        addRowLine(RightSide, rowData, &rightLineNumber);
                        blockNumber++;
                    }
                }

                if (j == contextFileData.chunks.size() - 1) {   // the last chunk
                    int skippedLines = -2;
                    if (chunkData.contextChunk) {
                        // if it's context chunk
                        skippedLines = chunkData.rows.size();
                    } else if (!contextFileData.lastChunkAtTheEndOfFile
                               && !contextFileData.contextChunksIncluded) {
                        // if not a context chunk and not a chunk at the end of file
                        // and context lines not included
                        skippedLines = -1;   // unknown count skipped by the end of file
                    }

                    if (skippedLines >= -1) {
                        addSkippedLine(LeftSide, skippedLines);
                        addSkippedLine(RightSide, skippedLines);
                        blockNumber++;
                    }   // otherwise nothing skipped
                }
            }
        }
        diffText[LeftSide].replace('\r', ' ');
        diffText[RightSide].replace('\r', ' ');
        output.side[LeftSide].diffText += diffText[LeftSide];
        output.side[RightSide].diffText += diffText[RightSide];
        if (isStop)
            return {};
    }
    output.side[LeftSide].selections = polishedSelections(output.side[LeftSide].selections);
    output.side[RightSide].selections = polishedSelections(output.side[RightSide].selections);
    return output;
}

int SideDiffData::blockNumberForFileIndex(int fileIndex) const
{
    if (fileIndex < 0 || fileIndex >= diffFileInfo.count())
        return -1;

    return std::next(diffFileInfo.constBegin(), fileIndex).key();
}

int SideDiffData::fileIndexForBlockNumber(int blockNumber) const
{
    int i = -1;
    for (auto it = diffFileInfo.cbegin(), end = diffFileInfo.cend(); it != end; ++it, ++i) {
        if (it.key() > blockNumber)
            break;
    }

    return i;
}

void SideDiffData::setLineNumber(int blockNumber, int lineNumber)
{
    const QString lineNumberString = QString::number(lineNumber);
    blockLineNumbers.insert(blockNumber, lineNumber);
    lineNumberDigits = qMax(lineNumberDigits, lineNumberString.size());
}

void SideDiffData::setFileInfo(int blockNumber, const DiffFileInfo &fileInfo)
{
    diffFileInfo[blockNumber] = fileInfo;
    setSeparator(blockNumber, true);
}
