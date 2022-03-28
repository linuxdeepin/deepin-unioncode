/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef WINDOWSERVICE_H
#define WINDOWSERVICE_H

#include <framework/framework.h>

#include <QMap>

class Core;
class AbstractAction;
class AbstractMenu;
class AbstractCentral;
class AbstractWidget;
class AbstractConsole;
namespace dpfservice {
class WindowService final : public dpf::PluginService, dpf::AutoServiceRegister<WindowService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowService)
public:
    static QString name();

    explicit WindowService(QObject *parent = nullptr);
    /*!
     * \brief addNavAction 添加导航栏action
     * \param action 实例对象
     */
    DPF_INTERFACE(void, addNavAction, AbstractAction *action);

    /*!
     * \brief addMenu 添加菜单项
     * \param menu 实例对象
     */
    DPF_INTERFACE(void, addMenu, AbstractMenu *menu);

    /*!
     * \brief addAction 添加到其他由框架发布的可扩展menu
     * \param menuName 框架扩展menu发布名称
     * \param action 实例对象
     */
    DPF_INTERFACE(void, addAction, const QString &menuName, AbstractAction *action);

    /*!
     * \brief addCentral 添加中心显示组件
     * \param navName 导航栏名称，从AbstractAction获取
     * \param central 添加的居中组件实例对象
     */
    DPF_INTERFACE(void, addCentral, const QString &navName, AbstractCentral *central);

    /*!
     * \brief setEditTree 设置编辑器文件树
     * \param editTree 设置的编辑器文件树实例对象
     */
    DPF_INTERFACE(void, setEditorTree, AbstractWidget *editTree);

    /*!
     * \brief setEditorConsole 设置默认终端
     * \param console 实例对象
     */
    DPF_INTERFACE(void, setEditorConsole, AbstractConsole *console);

    /*!
     * \brief addContextWidget 添加交互组件
     * \param contextTab Tab名称
     * \param contextWidget 实例对象
     */
    DPF_INTERFACE(void, addContextWidget, const QString &contextTab, AbstractWidget *contextWidget);

    /*!
     * \brief addOptionWidget 添加功能项组件
     * \param optionName option名称
     * \param optionWidget option显示的组件
     */
    DPF_INTERFACE(void, addOptionWidget, const QIcon &icon, const QString &optionName, AbstractWidget *optionWidget);

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
     * \brief setWatchWidget 添加监视器组件
     * \param widget 抽象接口
     */
    DPF_INTERFACE(void, setWatchWidget, AbstractWidget *widget);

    /*!
     * \brief switchNavWidget
     * \param name
     */
    DPF_INTERFACE(void, switchNavWidget, const QString &name);

    /*!
     * \brief switchContextWidget
     * \param name
     */
    DPF_INTERFACE(void, switchContextWidget, const QString &name);
};

} // namespace dpfservice

#endif // WINDOWSERVICE_H
