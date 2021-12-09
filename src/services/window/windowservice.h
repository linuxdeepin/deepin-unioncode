#ifndef WINDOWSERVICE_H
#define WINDOWSERVICE_H

#include "base/abstractmainwindow.h"
#include <framework/framework.h>
#include <QMap>

class Core;
class AbstractAction;
class AbstractMenu;
class AbstractCentral;
class AbstractWidget;
namespace dpfservice {
class WindowService final : public dpf::PluginService, dpf::AutoServiceRegister<WindowService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowService)
public:
    static QString name();

    explicit WindowService(QObject *parent = nullptr);

signals: // publish to other plugin interface
    /*!
     * \brief addNavAction 添加导航栏action
     * \param action 实例对象
     */
    void addNavAction(AbstractAction *action);

    /*!
     * \brief addMenu 添加菜单项
     * \param menu 实例对象
     */
    void addMenu(AbstractMenu *menu);

    /*!
     * \brief addAction 添加到其他由框架发布的可扩展menu
     * \param menuName 框架扩展menu发布名称
     * \param action 实例对象
     */
    void addAction(const QString &menuName, AbstractAction *action);

    /*!
     * \brief addCentral 添加中心显示组件
     * \param navName 导航栏名称，从AbstractAction获取
     * \param central 添加的居中组件实例对象
     */
    void addCentral(const QString &navName, AbstractCentral *central);

    /*!
     * \brief setEditTree 设置编辑器文件树
     * \param editTree 设置的编辑器文件树实例对象
     */
    void setEditorTree(AbstractWidget *editTree);

    /*!
     * \brief setEditorConsole 设置默认终端
     * \param console 实例对象
     */
    void setEditorConsole(AbstractWidget *console);

    /*!
     * \brief addContextWidget 添加交互组件
     * \param contextTab Tab名称
     * \param contextWidget 实例对象
     */
    void addContextWidget(const QString &contextTab, AbstractWidget *contextWidget);
};
} // namespace dpfservice
#endif // WINDOWSERVICE_H
