// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <DFrame>
#include <DToolButton>

#include <QVBoxLayout>
#include <QHash>

using DTK_WIDGET_NAMESPACE::DToolButton;
class NavigationBar : public DTK_WIDGET_NAMESPACE::DFrame
{
public:
    enum itemPositioin {
        top,
        bottom
    };

    NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar();

    void addNavItem(QAction *action, itemPositioin pos = top, quint8 priority = 10);   //priority : 0 highest, 255 lowest
    void setNavActionChecked(const QString &actionName, bool checked);
    void updateUi();

private:
    QVBoxLayout *topLayout { nullptr };
    QVBoxLayout *bottomLayout { nullptr };

    DToolButton *createToolBtn(QAction *action, bool isNavigationItem);

    QHash<QString, DToolButton *> navBtns;
    QMap<quint8, QList<DToolButton *>> topBtnsByPriority;
    QMap<quint8, QList<DToolButton *>> bottomBtnsByPriority;
};

#endif   // NAVIGATIONBAR_H
