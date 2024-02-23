// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTLOCATOR_H
#define ABSTRACTLOCATOR_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QIcon>
#include <QKeySequence>

class abstractLocator;
struct baseLocatorItem
{
public:
    explicit baseLocatorItem(abstractLocator *parentLocator)
        : locator(parentLocator) {}

    QString displayName;
    QString id;
    QString extraInfo;
    QVariant data;
    QIcon icon;

    abstractLocator *locator { nullptr };

    bool operator==(const baseLocatorItem &other) const
    {
        return this->id == other.id;
    }
};

class abstractLocator : public QObject
{
public:
    abstractLocator(QObject *parent = nullptr);
    static QRegularExpression createRegExp(const QString &text);

    virtual void prepareSearch(const QString &searchText) = 0;
    virtual QList<baseLocatorItem> matchesFor(const QString &inputText) = 0;
    virtual void accept(baseLocatorItem item) = 0;

    virtual void refresh() {}

    QString getDisplayName();
    void setDisplayName(const QString &displayName);

    QString getDescription();
    void setDescription(const QString &description);

    QKeySequence getShortCut();
    void setShortCut(const QKeySequence &key);

    bool isIncluedByDefault();
    void setIncludedDefault(bool isDefault);

private:
    QString description { "" };
    QString displayName { "" };
    QKeySequence shortCut { "" };
    bool includedByDefault { false };
};

#endif   // ABSTRACTLOCATOR_H
