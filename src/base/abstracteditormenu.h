// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTEDITORMENU_H
#define ABSTRACTEDITORMENU_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
QT_END_NAMESPACE

class AbstractEditorMenu : public QObject
{
    Q_OBJECT
public:
    explicit AbstractEditorMenu(QObject *parent = nullptr);
    virtual ~AbstractEditorMenu();
    virtual QString name() const = 0;
    virtual bool initialize(const QVariantHash &params);
    virtual bool create(QMenu *parent);
    virtual void updateState(QMenu *parent);
    virtual bool triggered(QAction *action);
    virtual AbstractEditorMenu *menu(QAction *action) const;
    virtual bool addSubmenu(AbstractEditorMenu *menu);
    virtual void removeSubmenu(AbstractEditorMenu *scene);
    inline QList<AbstractEditorMenu *> submenu() const
    {
        return subMenu;
    }

protected:
    virtual void setSubmenu(const QList<AbstractEditorMenu *> &menus);

protected:
    QList<AbstractEditorMenu *> subMenu;
};

#endif // ABSTRACTEDITORMENU_H
