// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCREPLACEHWORKER_H
#define SEARCREPLACEHWORKER_H

#include "constants.h"

#include <QObject>

class SearchReplaceWorkerPrivate;
class SearchReplaceWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchReplaceWorker(QObject *parent = nullptr);
    ~SearchReplaceWorker();

    void stop();

    FindItemList getResults();

public Q_SLOTS:
    void addSearchTask(const SearchParams &params);
    void addReplaceTask(const ReplaceParams &params);

Q_SIGNALS:
    void matched();
    void searchFinished();
    void replaceFinished(int result);

private Q_SLOTS:
    void handleReadSearchResult(const QString &keyword, bool caseSensitive, bool wholeWords);
    void processDone(int jobType);

private:
    friend class SearchReplaceWorkerPrivate;
    SearchReplaceWorkerPrivate *const d;
};

#endif   // SEARCREPLACEHWORKER_H
