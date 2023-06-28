// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBARMANAGER_H
#define TOOLBARMANAGER_H

#include <QObject>
#include <QToolBar>

class ToolBarManagerPrivate;
class ToolBarManager : public QObject
{
    Q_OBJECT
public:
    explicit ToolBarManager(const QString &name, QObject *parent = nullptr);
    virtual ~ToolBarManager() override;

    bool addActionItem(const QString &id, QAction *action, const QString &group);
    bool addWidgetItem(const QString &id, QWidget *widget, const QString &group);
    bool hasOverrideActionItem(const QString &id, QAction *action, const QString &group);
    void removeItem(const QString &id);
    void disableItem(const QString &id, bool visible);
    QToolBar *getToolBar() const;

private:
    ToolBarManagerPrivate *const d;
};

#endif // TOOLBARMANAGER_H
