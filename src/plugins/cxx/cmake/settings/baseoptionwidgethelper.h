// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEOPTIONWIDGETHELPER_H
#define BASEOPTIONWIDGETHELPER_H

#include "global_define.h"

#include <QWidget>
#include <QStandardItem>
#include <QMenu>

enum OptionRole {
    OptionNameRole = Qt::UserRole + 1,
    OptionPathRole,
    OptionTypeRole,
    OptionValidRole
};

enum OptionType {
    Detected = 0,
    Manual
};

class BaseOptionWidgetHelperPrivate;
class BaseOptionWidgetHelper : public QWidget
{
    Q_OBJECT
public:
    explicit BaseOptionWidgetHelper(QWidget *parent = nullptr);
    ~BaseOptionWidgetHelper();

    virtual QPair<QString, QList<Option>> getOptions() const = 0;

    void addItem(QStandardItem *item);
    void setAddButtonMenu(QMenu *menu);
    void setState(bool enabled, bool clear);
    void checkAndSetItemValid(QStandardItem *item);
    void applyCurrentOption() const;
    void clearItem(QStandardItem *item);
    void expandAll();
    void sort(Qt::SortOrder order = Qt::AscendingOrder);

    QModelIndex indexFromItem(const QStandardItem *item);
    void setCurrentIndex(const QModelIndex &index);

Q_SIGNALS:
    void requestAddOption();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    BaseOptionWidgetHelperPrivate *const d;
};

#endif   // BASEOPTIONWIDGETHELPER_H
