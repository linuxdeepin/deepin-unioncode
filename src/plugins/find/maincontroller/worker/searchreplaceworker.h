// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHREPLACEWORKER_H
#define SEARCHREPLACEWORKER_H

#include "constants.h"

#include <QObject>

class SearchReplaceWorkerPrivate;
class SearchReplaceWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchReplaceWorker(QObject *parent = nullptr);
    ~SearchReplaceWorker();

    bool hasItem() const;
    FindItemList takeAll();

public Q_SLOTS:
    void stop();
    void search(const SearchParams &params);
    void replace(const ReplaceParams &params);

Q_SIGNALS:
    void matched();
    void finished();

private Q_SLOTS:
    void handleReadSearchResult(const QString &keyword, SearchFlags flags);
    void processDone();

private:
    friend class SearchReplaceWorkerPrivate;
    SearchReplaceWorkerPrivate *const d;
};

#endif   // SEARCHREPLACEWORKER_H
