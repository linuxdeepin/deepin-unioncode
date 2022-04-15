#include "projectservice.h"
namespace dpfservice {

extern const QString MWCWT_PROJECTS {QTabWidget::tr("Projects")};

dpfservice::ProjectService::ProjectService(QObject *parent)
    : dpf::PluginService (parent)
{

}

QString dpfservice::ProjectService::name()
{
    return "org.deepin.service.ProjectService";
}

/*!
 * \brief setToolKitName设置工具套件名称, createRootItem实现时内部方便使用
 * \param item 文件樹節點
 * \param toolkit 名稱
 */
void ProjectGenerator::setToolKitName(QStandardItem *item, const QString &name)
{
    if (!item)
        return;
    setToolKitProperty(item, ToolKitPropertyKey::get()->ToolKitName, name);
}

/*!
 * \brief setToolKitProperty 设置工具套件特性, createRootItem实现时内部方便使用
 * \param item 文件树节点
 * \param key 键
 * \param value 值
 */
QString ProjectGenerator::toolKitName(QStandardItem *item)
{
    return toolKitPropertyMap(item)[ToolKitPropertyKey::get()->ToolKitName].toString();
}

/*!
 * \brief setToolKitProperty 设置工具套件特性, createRootItem实现时内部方便使用
 * \param item 文件树节点
 * \param key 键
 * \param value 值
 */
void ProjectGenerator::setToolKitProperty(QStandardItem *item, const QString &key, const QVariant &value)
{
    if (!item)
        return;
    auto map = item->data(ToolKitProperty).toMap();
    map[key] = value;
    item->setData(map, ToolKitProperty);
}

/*!
 * \brief toolKitProperty 获取工具套件特性
 * \param item 文件树节点
 * \param key 键
 * \return 万能类型，需要知道存入方类型进行转换后使用
 */
QVariant ProjectGenerator::toolKitProperty(QStandardItem *item, const QString &key)
{
    if (!item)
        return {};
    return toolKitPropertyMap(item)[key];
}

/*!
 * \brief toolKiyPropertyMap 获取所有特性值
 * \param item 文件树节点
 * \return 存入所有的特性Map
 */
QVariantMap ProjectGenerator::toolKitPropertyMap(QStandardItem *item)
{
    if (!item)
        return {};
    return item->data(ToolKitProperty).toMap();
}

}

