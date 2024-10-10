// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ALLPROJECTFILELOCATOR_H
#define ALLPROJECTFILELOCATOR_H

#include "basefilelocator.h"
#include "common/util/fileutils.h"

#include <QObject>

class AllProjectFileLocator : public baseFileLocator
{
    Q_OBJECT
public:
    AllProjectFileLocator(QObject *parent = nullptr);

    void prepareSearch(const QString &searchText) override;
};

#endif   // ALLPROJECTFILELOCATOR_H
