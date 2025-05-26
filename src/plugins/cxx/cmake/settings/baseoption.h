// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEOPTION_H
#define BASEOPTION_H

#include <QVariantMap>

class BaseOption
{
public:
    virtual QString titleName() const = 0;
    virtual QString configName() const = 0;
    virtual void setConfig(const QVariantMap &config) = 0;
    virtual QVariantMap getConfig() const = 0;
};

#endif   // BASEOPTION_H
