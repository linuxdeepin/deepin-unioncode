// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWSERVICE_H
#define WINDOWSERVICE_H

#include "windowcontroller.h"
#include "windowelement.h"

#include <framework/framework.h>

#include <QMap>
#include <QDockWidget>

class Core;
class AbstractAction;
class AbstractMenu;
class AbstractWidget;

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
        : dpf::PluginService(parent)
    {
    }

    //--controller--
    /*!
     * \brief The plugin has a View, check if View is created
     * \param plugin Name
     */
    DPF_INTERFACE(bool, hasView, const QString &plugin);

    /*!
     * \brief The plugin has a View, raise it if it`s created
     * \param plugin Name
     */
    DPF_INTERFACE(void, raiseView, const QString &plugin);

    /*!
     * \brief mode can only be CM_EDIT / CM_RECENT / CM_DEBUG .
     * you can raise a mode , then replace and add/delete windows on it
     * \param mode Name ( CM_EDIT / CM_RECENT / CM_DEBUG
     */
    DPF_INTERFACE(void, raiseMode, const QString &mode);

    /*!
     * \brief Automatically set current plugin when switching navigation actions.
     * When adding or deleting windows without switching the navigation bar, need call this function,
     * or the current View will add a window which dont`t belong it`s plugin.
     * \param plugin Name
     */
    DPF_INTERFACE(void, setCurrentPlugin, const QString &plugin);

    /*!
     * \brief add Widget to Position, Effective immediately, can use this interface when action triggerd
     * \param widget name
     * \param abstractWidget
     * \param Position
     * \param replace
     */
    DPF_INTERFACE(void, addWidget, const QString &name, AbstractWidget *abstractWidget, Position pos, bool replace);

    /*!
     * \brief replace Widget to Position, Effective immediately, can use this interface when action triggerd
     * \param widget name
     * \param abstractWidget
     * \param Position
     * same as addWidget(*, *, *, true)
     */
    DPF_INTERFACE(void, replaceWidget, const QString &name, AbstractWidget *abstractWidget, Position pos);

    /*!
     * \brief insert Widget to Position, stay with current widget, Effective immediately, can use this interface when action triggerd
     * \param widget name
     * \param abstractWidget
     * \param Position
     * same as addWidget(*, *, *, false)
     */
    DPF_INTERFACE(void, insertWidget, const QString &name, AbstractWidget *abstractWidget, Position pos);

    /*!
     * \brief register Widget to mode, when switch to this mode, automatically show widget.
     * \param widget name
     * \param abstractWidget
     * \param mode ( CM_EDIT / CM_RECENT / CM_DEBUG
     * \param Position
     * \param replace
     * \param isVisible
     */
    DPF_INTERFACE(void, registerWidgetToMode, const QString &name, AbstractWidget *abstractWidget, const QString &mode, Position pos, bool replace, bool isVisible);

    /*!
     * \brief every widget added to controller will becomes a dockWidget(except window added to Central/fullwindow.
     * widget in dockWidget -> Dock(widget), when dock is hidden, call widget.show/widget.hide is useless
     * hideWidget/showWidget is show/hide the Dock.
     * \param widget name
     */
    DPF_INTERFACE(void, showWidget, const QString &name);

    /*!
     * \brief every widget added to controller will becomes a dockWidget(except window added to Central/fullwindow.
     * widget in dockWidget -> Dock(widget), when dock is hidden, call widget.show/widget.hide is useless
     * hideWidget/showWidget is show/hide the Dock.
     * \param widget name
     */
    DPF_INTERFACE(void, hideWidget, const QString &name);

    /*!
     * \brief add Widget to Position, Effective immediately, can use this interface when action triggerd
     * \param widget name
     * \param abstractWidget
     * \param Position
     * \param replace
     * \param orientation  eg. widget added to Left/Right default orientation is Horizontal. can use this interface to change it
     */
    DPF_INTERFACE(void, addWidgetByOrientation, const QString &name, AbstractWidget *widget, Position pos, bool replace, Qt::Orientation);

    /*!
     * \brief split two windows in the specified direction
     * \param first name
     * \param second name
     * \param orientation
     */
    DPF_INTERFACE(void, splitWidgetOrientation, const QString &first, const QString &second, Qt::Orientation orientation);

    DPF_INTERFACE(void, setDockWidgetFeatures, const QString &name, QDockWidget::DockWidgetFeatures feature);

    DPF_INTERFACE(void, addNavigationItem, AbstractAction *action);
    DPF_INTERFACE(void, addNavigationItemToBottom, AbstractAction *action);
    DPF_INTERFACE(void, switchWidgetNavigation, const QString &navName);

    /*!
     * \brief add widget to ContextWidget, the visible of contextWidget is controlled by controller.
     * \param title
     * \param contextWidget
     * \param isVisible
     */
    DPF_INTERFACE(void, addContextWidget, const QString &title, AbstractWidget *contextWidget, bool isVisible);
    DPF_INTERFACE(bool, hasContextWidget, const QString &title);

    /*!
     * \brief show/hide the contextWidget
     */
    DPF_INTERFACE(void, showContextWidget);
    DPF_INTERFACE(void, hideContextWidget);

    /*!
     * \brief switchContextWidget switch widget in contextWidget by title
     * \param title
     */
    DPF_INTERFACE(void, switchContextWidget, const QString &title);

    DPF_INTERFACE(void, addChildMenu, AbstractMenu *menu);

    /*!
    * \brief insertAction 添加Action到其他由框架发布的可扩展menu
    * \param menuName 框架扩展menu发布名称
    * \param beforActionName 之后的Action名称
    * \param action 实例对象
    */
    DPF_INTERFACE(void, insertAction, const QString &menuName, const QString &beforActionName, AbstractAction *action);

    DPF_INTERFACE(void, addAction, const QString &menuName, AbstractAction *action);

    DPF_INTERFACE(void, removeActions, const QString &menuName);

    /*!
     * \brief addOpenProjectAction 添加打開工程的Action
     * \param name 语言分类名称 MWMFA_CXX|MWMFA_Java|MWMFA_Python
     * \param action 實例對象
     */
    DPF_INTERFACE(void, addOpenProjectAction, const QString &name, AbstractAction *action);

    /*!
    * \brief addWidgetToTopTool  Adding widget to a toptoolbar
    * widgets belongs to a group, you can show toptoolbar by group in swtichWidget event.
    * or you can add it to MWTG_EDIT/MWTG_DEBUG, it will automatically show shen switch to Edit/Debug mode
    * \param abstractWidget
    * \param group
    * \param addSeparator separator in front of this widget
    * \param addToLeft
    */
    DPF_INTERFACE(void, addWidgetToTopTool, AbstractWidget *abstractWidget, const QString &group, bool addSeparator, bool addToLeft);

    DPF_INTERFACE(void, addTopToolItem, AbstractAction *action, const QString &group, bool addSeparator);
    DPF_INTERFACE(void, addTopToolItemToRight, AbstractAction *action, bool addSeparator);

    DPF_INTERFACE(void, showTopToolBar, const QString &group);
    DPF_INTERFACE(void, hideTopToolBar);

    DPF_INTERFACE(void, showStatusBar);
    DPF_INTERFACE(void, hideStatusBar);

    /*!
     * \brief addWidgetWorkspace
     * \param AbstractWidget
     * \param widget
     */
    DPF_INTERFACE(void, addWidgetWorkspace, const QString &title, AbstractWidget *widget, const QString &iconName);

    DPF_INTERFACE(void, switchWorkspaceArea, const QString &title);
};

}   // namespace dpfservice

#endif   // WINDOWSERVICE_H
