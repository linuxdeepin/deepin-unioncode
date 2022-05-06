#ifndef PROJECTGENERATOR_H
#define PROJECTGENERATOR_H

#include "common/common.h"

#include <framework/framework.h>

#include <QStandardItem>
#include <QMenu>

namespace dpfservice {

/*!
 * \brief The ProjectGenerator class 工程生成器对象
 * 该类主要功能为生成各类工程提供接口, 该类与ProjectService接口分开
 * 达到接口隔离和适应插件组合变化
 * 模式1. 仅仅包含ProjectGenerator的插件（独立功能的插件）
 * 模式2. 包含ProjectGenerator、主界面框架组件发布、流程执行（倾向于业务流程）
 * ProjectService 面向工程核心(ProjectCore) 提供对外接口
 * ProjectGenerator 面向工程扩展实现其他插件对外的接口
 * ProjectCore <-> ProjectService <-> [outside plugin]
 */
class ProjectGenerator : public Generator
{
    Q_OBJECT
public:
    /*!
     * \brief The ItemDataRole enum
     *  获得QModelIndex 与Item数据互通支持(Qt规范)
     *  关联数据为QVariantMap
     */
    enum ItemDataRole
    {
        RootToolKitRole = Qt::ItemDataRole::UserRole,
    };

    // ProjectToolKitRole 的关联数据结构键
    enum_def(RootToolKitKey, QString)
    {
        enum_exp KitName = "KitName";
        enum_exp BuildPath = "BuildPath";
        enum_exp SourcePath = "SourcePath";
        enum_exp WorkspacePath = "WorkspacePath";
        enum_exp FilePath = "FilePath";
    };

    ProjectGenerator(){}

    /*!
     * \brief configure 生成器配置界面，子类需要重载实现
     *  按照一般的做法，当前界面应当生成配置文件与工程路径绑定，
     *  在界面上选中、输入信息存入配置文件
     * \param projectPath 工程路径
     * \return 返回工程配置界面
     */
    virtual QWidget* configureWidget(const QString &projectPath) {
        Q_UNUSED(projectPath)
        return nullptr;
    }

    /*!
     * \brief configure 执行生成器的过程
     *  按照生成器配置界面约定规则，实现执行过程，
     *  例如在cmake中，需要执行configure指令生成cache等相关文件
     *  执行该函数，应当首先确保前置条件满足(配置文件已生成)。
     * \param projectPath 工程路径
     * \return 过程执行结果
     */
    virtual bool configure(const QString &projectPath) {
        Q_UNUSED(projectPath)
        return false;
    }

    /*!
     * \brief createRootItem 创建文件树路径，子类需要重载实现
     *  执行该函数应当首先确定前置条件的满足，比如已经执行了生成器的过程。
     * \param projectPath 工程文件路径
     * \return
     */
    virtual QStandardItem *createRootItem(const QString &projectPath, const QString &outputPath) {
        Q_UNUSED(projectPath);
        Q_UNUSED(outputPath);
        return nullptr;
    }

    /*!
     * \brief createIndexMenu 文件树index点击时触发，子类需要重载实现
     *  文件树中右键触发的创建Menu，传入树中QModelIndex，它与Item数据相通(Qt规范)
     *  QMenu对象在插件使用方会被释放
     * \param index 索引数据
     * \return
     */
    virtual QMenu* createItemMenu(const QStandardItem *item) {
        Q_UNUSED(item);
        return nullptr;
    }

    /*!
     * \brief setToolKitName 设置树节点的工具套件名称, 该函数传入顶层节点
     * \param root 顶层节点
     * \param name 套件名称
     */
    static void setToolKitName(QStandardItem *root,
                               const RootToolKitKey::type_value &name);

    /*!
     * \brief setToolKitProperty 设置工作套件附件字段
     * \param root 顶层节点
     * \param key 附加字段键
     * \param value 附加字段值
     */
    static void setToolKitProperty(QStandardItem *root,
                                   const RootToolKitKey::type_value &key,
                                   const QVariant &value);

    /*!
     * \brief toolKitName 获取套件名称
     * \param root 顶层节点
     * \return 套件名称
     */
    static QString toolKitName(const QStandardItem *root);
    static QString toolKitName(const QModelIndex &root);


    /*!
     * \brief toolKitProperty 获取工作套件附加字段
     * \param root 顶层节点
     * \param key 附加字段键
     * \return
     */
    static QVariant toolKitProperty(const QStandardItem *root,
                                    const RootToolKitKey::type_value &key);
    static QVariant toolKitProperty(const QModelIndex &root,
                                    const RootToolKitKey::type_value &key);

    /*!
     * \brief toolKitPropertyMap 获取所有附加字段
     * \param root 顶层节点
     * \return 所有附加的字段
     */
    static QVariantMap toolKitPropertyMap(const QStandardItem *root);
    static QVariantMap toolKitPropertyMap(const QModelIndex &root);

    /*!
     * \brief root 获取子项的顶层根节点
     * \param child 任意子节点
     * \return 顶层根节点
     */
    static const QStandardItem *root(const QStandardItem *child);
    static const QModelIndex root(const QModelIndex &child);

Q_SIGNALS:
    /*!
     * \brief targetExecute 工程目标执行指令.
     *  使用该信号, 由于当前插件不隶属于构建系统,需要构建生成并且统计构建错误，则需要与builder插件通信
     * \param program 执行程序
     * \param arguments 参数
     */
    void targetExecute(const QString &program, const QStringList &arguments);
};


} // namespace dpfservice

#endif // PROJECTGENERATOR_H
