// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWSERVICE_H
#define WINDOWSERVICE_H

#include "windowelement.h"

#include <framework/framework.h>

#include <QMap>

class Core;
class AbstractAction;
class AbstractMenu;
class AbstractCentral;
class AbstractWidget;
class AbstractConsole;

namespace dpfservice {
// service interface
class WindowService final : public dpf::PluginService, dpf::AutoServiceRegister<WindowService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowService)
public:
    static QString name()
    {
        return "org.deepin.service.WindowService";
    }

    explicit WindowService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    /*!
     * \brief addActionNavigation 添加导航栏action
     * \param action 实例对象
     */
    DPF_INTERFACE(void, addNavigation, const QString &navName, const QString &iconName);

    /*!
     * \brief addMenu 添加菜单项
     * \param menu 实例对象
     */
    DPF_INTERFACE(void, addMenu, AbstractMenu *menu);

    /*!
     * \brief addAction 添加Action到其他由框架发布的可扩展menu
     * \param menuName 框架扩展menu发布名称
     * \param action 实例对象
     */
    DPF_INTERFACE(void, addAction, const QString &menuName, AbstractAction *action);

    /*!
     * \brief removeActions
     * \param menuName 框架扩展menu发布名称
     */
    DPF_INTERFACE(void, removeActions, const QString &menuName);

    /*!
     * \brief insertAction 添加Action到其他由框架发布的可扩展menu
     * \param menuName 框架扩展menu发布名称
     * \param beforActionName 之后的Action名称
     * \param action 实例对象
     */
    DPF_INTERFACE(void, insertAction, const QString &menuName,
                                      const QString &beforActionName,
                                      AbstractAction *action);
    /*!
     * \brief addCentralNavigation 添加中心显示组件
     * \param name 导航栏名称，从AbstractAction获取
     * \param central 添加的居中组件实例对象
     */
    DPF_INTERFACE(void, addCentralNavigation, const QString &name, AbstractCentral *central);

    /*!
     * \brief addWidgetWorkspace 左侧工作空间组件,可以是文件树,工程管理等
     * \param title 设置
     * \param widget 设置的编辑器文件树实例对象
     */
    DPF_INTERFACE(void, addWidgetWorkspace, const QString &title, AbstractWidget *widget, const QString &iconName);

    /*!
     * \brief setWidgetConsole 设置默认终端
     * \param console 实例对象
     */
    DPF_INTERFACE(void, setWidgetConsole, AbstractConsole *console);

    /**
     * @brief DPF_INTERFACE
     * @param widget
     */
    DPF_INTERFACE(QWidget*, setWidgetEdit, AbstractCentral *widget);

    /*!
     * \brief addContextWidget 添加交互组件
     * \param contextTab Tab名称
     * \param contextWidget 实例对象
     */
    DPF_INTERFACE(void, addContextWidget, const QString &contextTab, AbstractWidget *contextWidget, const QString &group, bool isVisible);

    DPF_INTERFACE(bool, hasContextWidget, const QString &title);

    /**
     * @brief DPF_INTERFACE
     * @param contextWidget
     */
    DPF_INTERFACE(void, removeContextWidget, AbstractWidget *contextWidget);

    /*!
     * \brief setStatusBar
     * \param message
     */
    DPF_INTERFACE(void, setStatusBar, AbstractWidget *statusBar);

    /*!
     * \brief addWidgetToStatusBar
     * \param widget
     * \param stretch
     */
    DPF_INTERFACE(void, addWidgetToStatusBar, QWidget *widget);

    /*!
     * \brief insertWidgetToStatusBar
     * \param index
     * \param widget
     */
    DPF_INTERFACE(int, insertWidgetToStatusBar, int index, QWidget *widget);

    /*!
     * \brief setWidgetWatch 添加监视器组件
     * \param widget 抽象接口
     */
    DPF_INTERFACE(QWidget*, setWidgetWatch, AbstractWidget *widget);

    /*!
     * \brief addOpenProjectAction 添加打開工程的Action
     * \param name 语言分类名称 MWMFA_CXX|MWMFA_Java|MWMFA_Python
     * \param action 實例對象
     */
    DPF_INTERFACE(void, addOpenProjectAction, const QString &name, AbstractAction *action);

    /*!
     * \brief addFindToolBar
     * \param widget FindToolBar Widget
     */

    DPF_INTERFACE(void, addFindToolBar, AbstractWidget *widget);

    /*!
     * \brief showFindToolBar
     */
    DPF_INTERFACE(void, showFindToolBar);

    /*!
     * \brief addToolBarWidgetItem
     */
    DPF_INTERFACE(bool, addToolBarWidgetItem, const QString &id, AbstractWidget *widget, const QString &group);

    /*!
     * \brief addToolBarSeparator
     */
    DPF_INTERFACE(void, addToolBarSeparator, const QString &group);

    /*!
     * \brief removeToolBarItem
     */
    DPF_INTERFACE(void, removeToolBarItem, const QString &id);

    /*!
     * \brief setToolBarItemDisable
     */
    DPF_INTERFACE(void, setToolBarItemDisable, const QString &id, bool disable);

    DPF_INTERFACE(void, addTopToolBar, const QString &name, QAction *action, const QString &group, bool isSeparat);

    DPF_INTERFACE(AbstractCentral *, getCentralNavigation, const QString &navName);

    DPF_INTERFACE(void, addWorkspaceArea, const QString &title, AbstractWidget *widget);

    DPF_INTERFACE(void, switchWorkspaceArea, const QString &title);

    DPF_INTERFACE(void, switchWidgetNavigation, const QString &title);

    DPF_INTERFACE(void, insertToLeftBarBottom, AbstractWidget *toolBtn);

signals:

    /*!
     * \brief showMessageDisplay 展示处理消息提示框
     */
    void showMessageDisplay();

    /*!
     * \brief appendProcessMessage 添加处理消息
     * \param mess 消息
     * \param currentPercent 当前百分比
     * \param maxPercent 最大的百分比
     */
    void appendProcessMessage(const QString &mess, int currentPercent, int maxPrecent = 100);

    /*!
     * \brief hideMessageDisplay 隐藏处理消息提示框
     */
    void hideMessageDisplay();
};

} // namespace dpfservice

#endif // WINDOWSERVICE_H
