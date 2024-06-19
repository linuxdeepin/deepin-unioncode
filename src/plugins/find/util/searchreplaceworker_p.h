// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHREPLACEWORKER_P_H
#define SEARCHREPLACEWORKER_P_H

#include "searchreplaceworker.h"

#include "services/editor/editorservice.h"

class SearchReplaceWorkerPrivate : public QObject
{
    Q_OBJECT
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
        QString keyword;
        bool caseSensitive = false;
        bool wholeWords = false;
    };

    explicit SearchReplaceWorkerPrivate(SearchReplaceWorker *qq);
    ~SearchReplaceWorkerPrivate();

    void startAll();
    void startNextJob();

    void createSearchJob(const SearchParams &params);
    void createReplaceJob(const ReplaceParams &params);
    Job buildSearchJob(const QStringList &fileList,
                       const QStringList &includeList,
                       const QStringList &excludeList,
                       const QString &keyword,
                       SearchFlags flags,
                       bool isOpenedFile);
    Job buildReplaceJob(const QStringList &fileList,
                        const QString &oldText,
                        const QString &newText,
                        SearchFlags flags,
                        bool isOpenedFile);

    void processWorkingFiles(QStringList &baseFiles, QStringList &openedFiles);
    Q_INVOKABLE void replaceOpenedFile(const QString &fileName, const QString &oldText,
                                       const QString &newText, bool caseSensitive, bool wholeWords);

public:
    SearchReplaceWorker *q;

    dpfservice::EditorService *editSrv { nullptr };
    QMutex mutex;
    FindItemList searchResults;
    std::unique_ptr<QProcess> process { nullptr };

    QList<Job> jobList;
    int currentJob = 0;
};

#endif   // SEARCHREPLACEWORKER_P_H
