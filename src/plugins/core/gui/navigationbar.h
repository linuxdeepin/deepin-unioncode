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
    Q_OBJECT
public:
    enum itemPositioin {
        top,
        bottom
    };

    NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar();

    void addNavItem(QAction *action, itemPositioin pos = top, quint8 priority = 10);   //priority : 0 highest, 255 lowest
    void addNavButton(Dtk::Widget::DToolButton *button, itemPositioin pos = top, quint8 priority = 10);
    void setNavActionChecked(const QString &actionName, bool checked);
    QStringList getAllNavigationItemName();
    quint8 getPriorityOfNavigationItem(const QString &name);
    void updateUi();

signals:
    void enter();
    void leave();

protected:
    void enterEvent(QEvent *event) override { emit enter(); }
    void leaveEvent(QEvent *event) override { emit leave(); }

private:
    QVBoxLayout *topLayout { nullptr };
    QVBoxLayout *bottomLayout { nullptr };

    DTK_WIDGET_NAMESPACE::DToolButton *createToolBtn(QAction *action, bool isNavigationItem);

    QHash<QString, DTK_WIDGET_NAMESPACE::DToolButton *> navBtns;
    QMap<quint8, QList<DTK_WIDGET_NAMESPACE::DToolButton *>> topBtnsByPriority;
    QMap<quint8, QList<DTK_WIDGET_NAMESPACE::DToolButton *>> bottomBtnsByPriority;
};

#endif   // NAVIGATIONBAR_H
