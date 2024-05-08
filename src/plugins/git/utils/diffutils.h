// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIFFUTILS_H
#define DIFFUTILS_H

#include "constants.h"

#include "differ.h"

#include <QTextCharFormat>
#include <QObject>
#include <QMap>

class DiffFileInfo
{
public:
    enum PatchBehaviour {
        PatchFile,
        PatchEditor
    };

    DiffFileInfo() = default;
    DiffFileInfo(const QString &file, const QString &type = {})
        : fileName(file), typeInfo(type) {}
    QString fileName;
    QString typeInfo;
    PatchBehaviour patchBehaviour = PatchFile;
};

using DiffFileInfoArray = std::array<DiffFileInfo, SideCount>;

class DiffChunkInfo
{
public:
    int chunkIndexForBlockNumber(int blockNumber) const;
    int chunkRowForBlockNumber(int blockNumber) const;
    int chunkRowsCountForBlockNumber(int blockNumber) const;

    void setChunkIndex(int startBlockNumber, int blockCount, int chunkIndex)
    {
        diffChunkInfo.insert(startBlockNumber, { blockCount, chunkIndex });
    }

private:
    // start block number, block count of a chunk, chunk index inside a file.
    QMap<int, QPair<int, int>> diffChunkInfo;
};

class TextLineData
{
public:
    enum TextLineType {
        TextLine,
        Separator,
        Invalid
    };
    TextLineData() = default;
    TextLineData(const QString &txt)
        : text(txt), textLineType(TextLine) {}
    TextLineData(TextLineType t)
        : textLineType(t) {}
    QString text;
    /*
     * <start position, end position>
     * <-1, n> means this is a continuation from the previous line
     * <n, -1> means this will be continued in the next line
     * <-1, -1> the whole line is a continuation (from the previous line to the next line)
     */
    QMap<int, int> changedPositions;   // counting from the beginning of the line
    TextLineType textLineType = Invalid;
};

class RowData
{
public:
    RowData() = default;
    RowData(const TextLineData &l)
        : line({ l, l }), equal(true) {}
    RowData(const TextLineData &l, const TextLineData &r)
        : line({ l, r }) {}
    std::array<TextLineData, SideCount> line {};
    bool equal = false;
};

class ChunkData
{
public:
    QList<RowData> rows;
    QString contextInfo;
    std::array<int, SideCount> startingLineNumber {};
    bool contextChunk = false;
};

class FileData
{
public:
    enum FileOperation {
        ChangeFile,
        ChangeMode,
        NewFile,
        DeleteFile,
        CopyFile,
        RenameFile
    };

    FileData() = default;
    FileData(const ChunkData &chunkData) { chunks.append(chunkData); }
    QList<ChunkData> chunks;
    DiffFileInfoArray fileInfo {};
    FileOperation fileOperation = ChangeFile;
    bool binaryFiles = false;
    bool lastChunkAtTheEndOfFile = false;
    bool contextChunksIncluded = false;
};

class DiffSelection
{
public:
    QTextCharFormat format;
    int start = -1;
    int end = -1;
};

// block number, list of ranges
// DiffSelection.start - can be -1 (continues from the previous line)
// DiffSelection.end - can be -1 (spans to the end of line, even after the last character in line)
using DiffSelections = QMap<int, QList<DiffSelection>>;

class SideDiffData
{
public:
    DiffChunkInfo diffChunkInfo;
    // block number, fileInfo. Set for file lines only.
    QMap<int, DiffFileInfo> diffFileInfo;
    // block number, visual line number.
    QMap<int, int> blockLineNumbers;
    // block number, skipped lines and context info. Set for chunk lines only.
    QMap<int, QPair<int, QString>> blockSkippedLines;
    // block number, separator. Set for file, chunk or span line.
    QMap<int, bool> blockSeparators;

    int lineNumberDigits = 1;

    bool isFileLine(int blockNumber) const { return diffFileInfo.contains(blockNumber); }
    bool isChunkLine(int blockNumber) const { return blockSkippedLines.contains(blockNumber); }
    int blockNumberForFileIndex(int fileIndex) const;
    int fileIndexForBlockNumber(int blockNumber) const;

    void setLineNumber(int blockNumber, int lineNumber);
    void setFileInfo(int blockNumber, const DiffFileInfo &fileInfo);
    void setSkippedLines(int blockNumber, int skippedLines, const QString &contextInfo = {})
    {
        blockSkippedLines[blockNumber] = { skippedLines, contextInfo };
        setSeparator(blockNumber, true);
    }
    void setSeparator(int blockNumber, bool separator) { blockSeparators[blockNumber] = separator; }
};

class SideDiffOutput
{
public:
    SideDiffData diffData;
    QString diffText;
    DiffSelections selections;
};

class SideBySideDiffOutput
{
public:
    std::array<SideDiffOutput, SideCount> side {};
    // 'foldingIndent' is populated with <block number> and folding indentation
    // value where 1 indicates start of new file and 2 indicates a diff chunk.
    // Remaining lines (diff contents) are assigned 3.
    QHash<int, int> foldingIndent;
};

class SideBySideShowResult
{
public:
    QSharedPointer<QTextDocument> textDocument {};
    SideDiffData diffData;
    DiffSelections selections;
};

using SideBySideShowResults = std::array<SideBySideShowResult, SideCount>;

class DiffEditorInput
{
public:
    explicit DiffEditorInput(const QList<FileData> &dataList, const QFont &font);
    void setFormat(const QFont &font);

    QList<FileData> contextFileData;
    QTextCharFormat fileLineFormat;
    QTextCharFormat chunkLineFormat;
    QTextCharFormat spanLineFormat;
    std::array<QTextCharFormat, SideCount> lineFormat {};
    std::array<QTextCharFormat, SideCount> charFormat {};
};

class GitDiffWorker : public QObject
{
    Q_OBJECT
public:
    explicit GitDiffWorker(QObject *parent = nullptr);

    void stop();
    QList<FileData> getPatchResult() const;
    SideBySideShowResults getShowResults() const;

public Q_SLOTS:
    void handleParsePatch(const QString &patch);
    void handleParseFileDatas(const QList<FileData> &dataList, const QFont &font);

Q_SIGNALS:
    void parsePatchFinished();
    void parseFileDatasFinished(bool success);

private:
    QList<ChunkData> readChunks(const QString &patch, bool *lastChunkAtTheEndOfFile, bool *ok);
    void readGitPatch(QList<FileData> &dataList, const QString &patch);
    void readPatch(QList<FileData> &dataList, const QString &patch);
    void readDiffPatch(QList<FileData> &dataList, const QString &patch);
    FileData readDiffHeaderAndChunks(const QString &headerAndChunks, bool *ok);
    ChunkData calculateOriginalData(const QList<Diff> &leftDiffList,
                                    const QList<Diff> &rightDiffList);

    bool detectChunkData(const QString &chunkDiff, ChunkData *chunkData, QString *remainingPatch);
    QString readLine(const QString &text, QString *remainingText, bool *hasNewLine);
    QList<RowData> readLines(const QString &patch, bool lastChunk, bool *lastChunkAtTheEndOfFile, bool *ok);
    bool detectFileData(const QString &patch, FileData *fileData, QString *remainingPatch);
    bool extractCommonFileName(const QString &fileNames, QString *fileName);
    bool detectIndexAndBinary(const QString &patch, FileData *fileData, QString *remainingPatch);
    QString sideFileName(DiffSide side, const FileData &fileData);
    void handleDifference(const QString &text, QList<TextLineData> *lines, int *lineNumber);
    void handleLine(const QStringList &newLines, int line, QList<TextLineData> *lines, int *lineNumber);
    bool lastLinesEqual(const QList<TextLineData> &leftLines, const QList<TextLineData> &rightLines);
    QList<TextLineData> assemblyRows(const QList<TextLineData> &lines, const QMap<int, int> &lineSpans);
    SideBySideDiffOutput diffOutput(const DiffEditorInput &input);

private:
    QAtomicInteger<bool> isStop { false };
    QList<FileData> fileDataList;
    SideBySideShowResults showResults;
};

Q_DECLARE_METATYPE(FileData)
Q_DECLARE_METATYPE(QList<FileData>)

#endif   // DIFFUTILS_H
