#include "projectservice.h"
namespace dpfservice {

dpfservice::ProjectService::ProjectService(QObject *parent)
    : dpf::PluginService (parent)
{

}

QString dpfservice::ProjectService::name()
{
    return "org.deepin.service.ProjectService";
}

void ProjectGenerator::setToolKitName(QStandardItem *item, const QString &name)
{
    if (!item)
        return;
    setToolKitProperty(item, ToolKitPropertyKey::get()->ToolKitName, name);
}

QString ProjectGenerator::toolKitName(QStandardItem *item)
{
    return toolKitPropertyMap(item)[ToolKitPropertyKey::get()->ToolKitName].toString();
}

void ProjectGenerator::setToolKitProperty(QStandardItem *item, const QString &key, const QVariant &value)
{
    if (!item)
        return;
    auto map = item->data(ToolKitProperty).toMap();
    map[key] = value;
    item->setData(map, ToolKitProperty);
}

QVariant ProjectGenerator::toolKitProperty(QStandardItem *item, const QString &key)
{
    if (!item)
        return {};
    return toolKitPropertyMap(item)[key];
}

QVariantMap ProjectGenerator::toolKitPropertyMap(QStandardItem *item)
{
    if (!item)
        return {};
    return item->data(ToolKitProperty).toMap();
}

}

