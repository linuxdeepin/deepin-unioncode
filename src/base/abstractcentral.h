// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTCENTRAL_H
#define ABSTRACTCENTRAL_H

class AbstractCentralPrivate;
class AbstractCentral
{
    AbstractCentralPrivate *const d;
public:
    AbstractCentral(void *qwidget);
    virtual ~AbstractCentral();
    void *qWidget();
};

#endif // ABSTRACTCENTRAL_H
