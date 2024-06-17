// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEFILELOCATOR_H
#define BASEFILELOCATOR_H

#include "abstractlocator.h"
#include "common/util/fileutils.h"

#include <QObject>
#include <memory> // For using std::unique_ptr


// File Locator Item (derived from baseLocatorItem)
struct fileLocatorItem : public baseLocatorItem
{
    explicit fileLocatorItem(abstractLocator* parentLocator)
        : baseLocatorItem(parentLocator) {}

    Utils::FileName filePath;
};

class baseFileLocator : public abstractLocator
{
    Q_OBJECT
public:
    explicit baseFileLocator(QObject* parent = nullptr);
    virtual ~baseFileLocator() = default;  // Virtual destructor

    // Reimplement virtual methods from the base class
    void prepareSearch(const QString& searchText) override;
    QList<baseLocatorItem> matchesFor(const QString& inputText) const override;
    void accept(const baseLocatorItem& item) override; // Pass item by const reference

    // Specific methods for file locator
    void setFileList(const QList<QString>& fileList);
    void clear();

private:
    class BaseFileLocatorPrivate; // Forward declare private implementation class
    std::unique_ptr<BaseFileLocatorPrivate> d; // Use unique_ptr for memory management

    QString toShortProjectPath(const QString& path) const; 
};

#endif  // BASEFILELOCATOR_H
