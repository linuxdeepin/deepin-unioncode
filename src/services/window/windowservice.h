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
// window display elements
// MW = MainWindow
extern const int MW_MIN_WIDTH;
extern const int MW_MIN_HEIGHT;

// MWNA = MainWindow Navtion Action
extern const QString MWNA_RECENT;
extern const QString MWNA_EDIT;
extern const QString MWNA_DEBUG;
extern const QString MWNA_RUNTIME;

// MWM_FILE: MainWindow Menu File
// MWMFA: MainWindow Menu File Action
extern const QString MWM_FILE;
extern const QString MWMFA_DOCUMENT_NEW;
extern const QString MWMFA_FOLDER_NEW;
extern const QString MWMFA_OPEN_PROJECT;
extern const QString MWMFA_OPEN_DOCUMENT;
extern const QString MWMFA_OPEN_FOLDER;
extern const QString MWMFA_OPEN_RECENT_DOCUMENTS;
extern const QString MWMFA_OPEN_RECENT_FOLDER;
extern const QString MWMFA_QUIT;

// MWMB: MainWindow Menu Build
extern const QString MWM_BUILD;

// MWMB: MainWindow Menu Debug
extern const QString MWM_DEBUG;

// MWMT: MainWindow Menu Tools
extern const QString MWM_TOOLS;
extern const QString MWMTA_SEARCH;
extern const QString MWMTA_PACKAGE_TOOLS;
extern const QString MWMTA_VERSION_TOOLS;
extern const QString MWMTA_CODE_FORMATTING;
extern const QString MWMTA_RUNTIME_ANALYSIS;
extern const QString MWMTA_TEST;
extern const QString MWMTA_PLUGINS;
extern const QString MWMTA_OPTIONS;

// MWMH: MainWindow Menu Help
extern const QString MWM_HELP;

// Dialog Title text
extern const QString DIALOG_OPEN_DOCUMENT_TITLE;
extern const QString DIALOG_OPEN_FOLDER_TITLE;

// TabWidget Tab Text from editor
extern const QString CONSOLE_TAB_TEXT; // text's "Console"


// service interface
class WindowService final : public dpf::PluginService, dpf::AutoServiceRegister<WindowService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowService)
public:
    static QString name();

    explicit WindowService(QObject *parent = nullptr);
    /*!
     * \brief addActionNavigation 添加导航栏action
     * \param action 实例对象
     */
    DPF_INTERFACE(void, addActionNavigation, AbstractAction *action);

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
    DPF_INTERFACE(void, addWidgetWorkspace, const QString &title, AbstractWidget *widget);

    /*!
     * \brief setWidgetConsole 设置默认终端
     * \param console 实例对象
     */
    DPF_INTERFACE(void, setWidgetConsole, AbstractConsole *console);

    /*!
     * \brief addContextWidget 添加交互组件
     * \param contextTab Tab名称
     * \param contextWidget 实例对象
     */
    DPF_INTERFACE(void, addContextWidget, const QString &contextTab, AbstractWidget *contextWidget);

    /*!
     * \brief addWidgetOption 添加功能项组件
     * \param optionName option名称
     * \param optionWidget option显示的组件
     */
    DPF_INTERFACE(void, addWidgetOption, const QIcon &icon, const QString &optionName, AbstractWidget *optionWidget);

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
    DPF_INTERFACE(void, setWidgetWatch, AbstractWidget *widget);

    /*!
     * \brief switchWidgetNavigation 切换导航栏组件
     * \param name 导航栏组件名称（来自 Action Navigation）
     */
    DPF_INTERFACE(void, switchWidgetNavigation, const QString &name);

    /*!
     * \brief switchContextWidget 切换上下文组件
     * \param name 上下文组件名称
     */
    DPF_INTERFACE(void, switchWidgetContext, const QString &name);

    /*!
     * \brief switchWidgetLeftTree 切换左侧树图
     * \param name 树图名称
     */
    DPF_INTERFACE(void, switchWidgetWorkspace, const QString &name);

    /*!
     * \brief addOpenProjectAction 添加打開工程的Action
     * \param action 實例對象
     */
    DPF_INTERFACE(void, addOpenProjectAction, AbstractAction *action);

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
