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
    struct Job
    {
        QString program;
        QStringList arguments;
        QString channelData;
        QString keyword;
        SearchFlags flags;
    };

    explicit SearchReplaceWorkerPrivate(SearchReplaceWorker *qq);

    void startAll();
    void startNextJob();

    void createSearchJob(const SearchParams &params);
    Job buildSearchJob(const QStringList &fileList,
                       const QStringList &includeList,
                       const QStringList &excludeList,
                       const QString &keyword,
                       SearchFlags flags,
                       bool isOpenedFile);

    void parseResultWithRegExp(const QString &fileName, const QString &keyword, const QString &contents, int line, SearchFlags flags);
    void parseResultWithoutRegExp(const QString &fileName, const QString &keyword, const QString &contents, int line, SearchFlags flags);

    void processWorkingFiles(QStringList &baseFiles, QStringList &openedFiles);
    void replaceLocalFile(const QString &fileName, const QString &replacement, const FindItemList &itemList);
    Q_INVOKABLE void replaceOpenedFile(const QString &fileName, const QString &replacement, const FindItemList &itemList);

public:
    SearchReplaceWorker *q;

    dpfservice::EditorService *editSrv { nullptr };
    QMutex mutex;
    FindItemList searchResults;
    std::unique_ptr<QProcess> process { nullptr };

    QAtomicInteger<bool> isStop { false };
    QList<Job> jobList;
    int currentJob = 0;
    int resultCount = 0;
};

#endif   // SEARCHREPLACEWORKER_P_H
