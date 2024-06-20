// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTNAVACTION_H
#define ABSTRACTNAVACTION_H

#include <functional>

#include <QKeySequence>
#include <QAction>

class AbstractActionPrivate;
class AbstractAction
{
    AbstractActionPrivate *const d;
public:
    explicit AbstractAction(QAction *qAction);
    virtual ~AbstractAction();
    void setShortCutInfo(const QString &id, const QString &description, const QKeySequence defaultShortCut = QKeySequence());
    bool hasShortCut();
    QString id();
    QString description();
    QKeySequence keySequence();

    QAction *qAction();
};

#endif // ABSTRACTNAVACTION_H
