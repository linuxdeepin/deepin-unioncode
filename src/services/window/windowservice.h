// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWSERVICE_H
#define WINDOWSERVICE_H

#include "windowcontroller.h"
#include "windowelement.h"
#include "common/find/abstractdocumentfind.h"

#include <framework/framework.h>

#include <DToolButton>

#include <QMap>
#include <QDockWidget>

class Core;
class AbstractAction;
class AbstractMenu;
class AbstractWidget;
class AbstractInstaller;

namespace Priority {
const quint8 lowest = 255;
const quint8 low = 150;
const quint8 medium = 100;
const quint8 high = 50;
const quint8 highest = 5;
}

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
     * \brief mode can only be CM_EDIT / CM_RECENT / CM_DEBUG .
     * you can raise a mode , then replace and add/delete windows on it
     * \param mode Name ( CM_EDIT / CM_RECENT / CM_DEBUG
     */
    DPF_INTERFACE(void, raiseMode, const QString &mode);

    /*!
    * \brief replace Widget to Position, can use this interface when action triggerd
    * \param widget name
    * \param Position
    */
    DPF_INTERFACE(void, replaceWidget, const QString &name, Position pos);

    /*!
    * \brief insert Widget to Position, stay with current widget
    * \param widget name
    * \param Position
    * \param orientation Choose the insertion orientation,
    *  by default, Position::Left / Right : Orientation::Vertical
    *  Position::Top / Bottom : Orientation::Horizontal
    */
    DPF_INTERFACE(void, insertWidget, const QString &name, Position pos, Qt::Orientation orientation);

    DPF_INTERFACE(void, hideWidget, const QString &name);

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
     * \brief register Widget to core, when switch to this mode, Manually show widget through Action triggered.
     * \param widget name
     * \param abstractWidget
     */
    DPF_INTERFACE(void, registerWidget, const QString &name, AbstractWidget *abstractWidget);
    DPF_INTERFACE(void, showWidgetAtPosition, const QString &name, Position pos, bool replace);
    DPF_INTERFACE(QString, getCentralWidgetName);
    DPF_INTERFACE(QStringList, getCurrentDockName, Position pos);
    DPF_INTERFACE(void, resizeDocks, const QList<QString> &docks, const QList<int> &sizes, Qt::Orientation orientation);

    /*!
     * \brief split two windows in the specified direction
     * \param first name
     * \param second name
     * \param orientation
     */
    DPF_INTERFACE(void, splitWidgetOrientation, const QString &first, const QString &second, Qt::Orientation orientation);

    DPF_INTERFACE(void, setDockWidgetFeatures, const QString &name, QDockWidget::DockWidgetFeatures feature);

    DPF_INTERFACE(void, setDockHeaderName, const QString &dockName, const QString &headerName);
    DPF_INTERFACE(void, setDockHeaderList, const QString &dockName, const QList<QAction *> &headers);
    DPF_INTERFACE(void, deleteDockHeader, const QString &name);
    DPF_INTERFACE(void, addToolBtnToDockHeader, const QString &name, DTK_WIDGET_NAMESPACE::DToolButton *btn);
    /*!
     * \brief add navigationItem to leftToolBar
     * \param action
     * \param priority 0~255  0 is highest. default:10
     */
    DPF_INTERFACE(void, addNavigationItem, AbstractAction *action, quint8 priority);
    DPF_INTERFACE(void, addNavigationItemToBottom, AbstractAction *action, quint8 priority);
    DPF_INTERFACE(void, switchWidgetNavigation, const QString &navName);
    DPF_INTERFACE(void, bindWidgetToNavigation, const QString &dockName, AbstractAction *action);
    DPF_INTERFACE(QStringList, getAllNavigationItemName);
    DPF_INTERFACE(quint8, getPriorityOfNavigationItem, const QString &itemName);

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
    * \param group  "MWTG_EDIT" "MWTG_DEBUG" choost to display in edit mode or debug mode
    * \param addSeparator separator in front of this widget
    * \param addToLeft
    */
    DPF_INTERFACE(void, addWidgetToTopTool, AbstractWidget *abstractWidget, bool addSeparator, bool addToLeft, quint8 priority);

    DPF_INTERFACE(void, addTopToolItem, AbstractAction *action, bool addSeparator, quint8 priority);
    DPF_INTERFACE(void, addTopToolItemToRight, AbstractAction *action, bool addSeparator, quint8 priority);
    DPF_INTERFACE(void, removeTopToolItem, AbstractAction *action);
    DPF_INTERFACE(void, switchTopToolItemVisible, AbstractAction *action, bool visible);

    DPF_INTERFACE(void, showTopToolBar);
    DPF_INTERFACE(void, hideTopToolBar);

    DPF_INTERFACE(void, showStatusBar);
    DPF_INTERFACE(void, hideStatusBar);
    DPF_INTERFACE(void, addStatusBarItem, QWidget *item);

    /*!
     * \brief addWidgetWorkspace if the widget support fold/expand, set widget.property("canExpand") to show fold/expand button
     * \param AbstractWidget
     * \param widget
     */
    DPF_INTERFACE(void, addWidgetWorkspace, const QString &title, AbstractWidget *widget, const QString &iconName);
    DPF_INTERFACE(void, registerToolBtnToWidget,const QString &dockName, Dtk::Widget::DToolButton *btn);
    DPF_INTERFACE(void, registerToolBtnToWorkspaceWidget, Dtk::Widget::DToolButton *btn, const QString &title);

    DPF_INTERFACE(void, switchWorkspaceArea, const QString &title);

    DPF_INTERFACE(QWidget *, createFindPlaceHolder, QWidget *owner, AbstractDocumentFind *docFind);

    /**
     * @brief notify
     * @param type 0-infomation, 1-warning, 2-error
     * @param name
     * @param msg
     * @param actions {id, text, id, text, ...}
     */
    DPF_INTERFACE(void, notify, uint type, const QString &name, const QString &msg, const QStringList &actions);
    using NotifyCallback = std::function<void(const QString &actId)>;
    DPF_INTERFACE(void, notifyWithCallback, uint type, const QString &name, const QString &msg, const QStringList &actions, NotifyCallback cb);

    DPF_INTERFACE(void, registerInstaller, const QString &name, AbstractInstaller *installer);
    /**
     * @param plugin: plugin name
     * @param name: installer name, with apt and pip by default
     * @param packageList: installation package list
     * @param error: error message
     */
    DPF_INTERFACE(bool, installPackages, const QString &plugin, const QString &name, const QStringList &packageList, QString *error);
};

}   // namespace dpfservice

#endif   // WINDOWSERVICE_H
