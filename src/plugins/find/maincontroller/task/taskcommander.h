// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_H
#define TASKCOMMANDER_H

#include "constants.h"

#include <QObject>

class TaskCommanderPrivate;
class TaskCommander : public QObject
{
    Q_OBJECT
public:
    explicit TaskCommander(QObject *parent = nullptr);

    bool search(const SearchParams &params);
    bool replace(const ReplaceParams &params);
    void stop();
    void deleteSelf();
    FindItemList takeAll();

Q_SIGNALS:
    void matched();
    void finished();

private:
    TaskCommanderPrivate *const d;
};

#endif   // TASKCOMMANDER_H
