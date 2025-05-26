// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEFILELOCATOR_H
#define BASEFILELOCATOR_H

#include "base/abstractlocator.h"
#include "common/util/fileutils.h"
#include "services/project/projectservice.h"

#include <QObject>

struct fileLocatorItem : public baseLocatorItem
{
    explicit fileLocatorItem(abstractLocator *parentLocator)
        : baseLocatorItem(parentLocator) {}

    Utils::FileName filePath;
};

class baseFileLocator : public abstractLocator
{
    Q_OBJECT
public:
    explicit baseFileLocator(QObject *parent = nullptr);

    void prepareSearch(const QString &searchText) override;
    QList<baseLocatorItem> matchesFor(const QString &inputText) override;
    void accept(baseLocatorItem item) override;

    void setFileList(const QList<QString> &fileList);
    void clear();

private:
    QList<fileLocatorItem> locatorItemList {};
    QList<QString> oldFileList {};
    QList<QString> fileList {};

    QString toShortProjectPath(const QString &path);
    dpfservice::ProjectService *projectService { nullptr };
};

#endif   // BASEFILELOCATOR_H
