// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTNAVACTION_H
#define ABSTRACTNAVACTION_H

#include <functional>
class AbstractActionPrivate;
class AbstractAction
{
    AbstractActionPrivate *const d;
public:
    explicit AbstractAction(void *qAction);
    virtual ~AbstractAction();
    void *qAction();
};

#endif // ABSTRACTNAVACTION_H
