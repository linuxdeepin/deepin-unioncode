// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CURRENTPROJECTLOCATOR_H
#define CURRENTPROJECTLOCATOR_H

#include "base/basefilelocator.h"
#include "common/util/fileutils.h"

#include <QObject>

class CurrentProjectLocator : public baseFileLocator
{
    Q_OBJECT
public:
    CurrentProjectLocator(QObject *parent = nullptr);

    void prepareSearch(const QString &searchText) override;
};

#endif // CURRENTPROJECTLOCATOR_H
