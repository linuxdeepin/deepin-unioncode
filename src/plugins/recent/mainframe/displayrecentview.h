// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISPLAYRECENTVIEW_H
#define DISPLAYRECENTVIEW_H

#include <DListView>
#include <QFileIconProvider>

class QStandardItemModel;
class QStandardItem;
class DisplayRecentView : public DTK_WIDGET_NAMESPACE::DListView
{
    Q_OBJECT
    QStringList cache;
    QFileIconProvider iconProvider;
public:
    explicit DisplayRecentView(QWidget *parent = nullptr);
    virtual QString cachePath();
    virtual void add(const QString &data);
    virtual QIcon icon(const QString &data);
    virtual QString title() = 0;
    virtual void load();
    virtual QList<QStandardItem*> itemsFromFile();
protected:
    virtual void saveToFile(const QStringList &cache);
protected:
    QStandardItemModel *model;
    QJsonDocument readRecent();
private:
};

#endif // DISPLAYRECENTVIEW_H
