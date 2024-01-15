// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <DFrame>
#include <DToolButton>

#include <QVBoxLayout>
#include <QHash>

class NavigationBar : public DTK_WIDGET_NAMESPACE::DFrame
{
public:
    enum itemPositioin{
        top,
        bottom
    };

    NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar();

    void addNavItem(QAction *action, itemPositioin pos = top);
    void setNavActionChecked(const QString &actionName, bool checked);

private:
    QVBoxLayout *topLayout { nullptr };
    QVBoxLayout *bottomLayout { nullptr };

    DTK_WIDGET_NAMESPACE::DToolButton *createToolBtn(QAction *action);

    QHash<QString, DTK_WIDGET_NAMESPACE::DToolButton*> navBtns;
};

#endif   // NAVIGATIONBAR_H
