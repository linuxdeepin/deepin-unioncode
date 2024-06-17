// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTLOCATOR_H
#define ABSTRACTLOCATOR_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QIcon>
#include <QKeySequence>
#include <memory> // For using std::unique_ptr

// Forward declare abstractLocator to avoid circular dependency
class abstractLocator;

// Structure for locator items
struct baseLocatorItem
{
public:
    explicit baseLocatorItem(abstractLocator* parentLocator) : locator(parentLocator) {}
    baseLocatorItem(const baseLocatorItem&) = default;  // Allow copying 

    QString displayName;
    QString id;
    QString extraInfo;
    QString tooltip;
    QVariant data;
    QIcon icon;

    abstractLocator* locator{nullptr}; // Parent locator

    bool operator==(const baseLocatorItem& other) const 
    {
        return this->id == other.id;
    }
};

class abstractLocator : public QObject
{
    Q_OBJECT // Add Q_OBJECT macro

public:
    explicit abstractLocator(QObject* parent = nullptr);
    virtual ~abstractLocator() = default;  // Virtual destructor 

    static QRegularExpression createRegExp(const QString& text); // Factory method for RegExp

    // Pure virtual methods (must be implemented in derived classes)
    virtual void prepareSearch(const QString& searchText) = 0;
    virtual QList<baseLocatorItem> matchesFor(const QString& inputText) const = 0;
    virtual void accept(const baseLocatorItem& item) = 0; // Pass item by const reference

    // Virtual method for refreshing data (can be overridden)
    virtual void refresh(); 

    // Getters and setters (now using const references)
    QString getDisplayName() const;
    void setDisplayName(const QString& displayName);
    QString getDescription() const;
    void setDescription(const QString& description);
    QKeySequence getShortCut() const;
    void setShortCut(const QKeySequence& key);

    bool isIncluedByDefault() const; // Corrected typo in method name
    void setIncludedDefault(bool isDefault);

signals:
    // You can add custom signals here (e.g., void searchFinished())

private:
    class AbstractLocatorPrivate; // Forward declare private implementation class
    std::unique_ptr<AbstractLocatorPrivate> d; // Use unique_ptr for automatic memory management
};

#endif  // ABSTRACTLOCATOR_H
