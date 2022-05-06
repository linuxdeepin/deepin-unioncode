#include "projectgenerator.h"

#include <QTabWidget>

namespace dpfservice {

// 自定义枚举，工具套件的唯一键
enum_def(ToolKitPropertyKey, QString)
{

};

void ProjectGenerator::setToolKitName(QStandardItem *root,
                                      const RootToolKitKey::type_value &name)
{
    if (!root)
        return;
    setToolKitProperty(root, RootToolKitKey::get()->KitName, name);
}

QString ProjectGenerator::toolKitName(const QStandardItem *root)
{
    return toolKitPropertyMap(root)[RootToolKitKey::get()->KitName].toString();
}

QString ProjectGenerator::toolKitName(const QModelIndex &root)
{
    return toolKitPropertyMap(root)[RootToolKitKey::get()->KitName].toString();
}

void ProjectGenerator::setToolKitProperty(QStandardItem *root,
                                          const RootToolKitKey::type_value &key,
                                          const QVariant &value)
{
    if (!root)
        return;
    auto map = root->data(RootToolKitRole).toMap();
    map[key] = value;
    root->setData(map, RootToolKitRole);
}

QVariant ProjectGenerator::toolKitProperty(const QStandardItem *root,
                                           const RootToolKitKey::type_value &key)
{
    if (!root)
        return {};
    return toolKitPropertyMap(root)[key];
}

QVariant ProjectGenerator::toolKitProperty(const QModelIndex &root,
                                           const RootToolKitKey::type_value &key)
{
    if (!root.isValid())
        return {};
    return toolKitPropertyMap(root)[key];
}

QVariantMap ProjectGenerator::toolKitPropertyMap(const QStandardItem *root)
{
    if (!root)
        return {};
    return root->data(RootToolKitRole).toMap();
}

QVariantMap ProjectGenerator::toolKitPropertyMap(const QModelIndex &root)
{
    if (!root.isValid())
        return {};
    return root.data(RootToolKitRole).toMap();
}

const QStandardItem *ProjectGenerator::root(const QStandardItem *child)
{
    if (!child)
        return nullptr;

    const QStandardItem *parent = child->parent();
    if (parent) {
        return root(parent);
    }
    return child;
}

const QModelIndex ProjectGenerator::root(const QModelIndex &child)
{
    if (!child.isValid())
        return {};

    const QModelIndex parent = child.parent();
    if (parent.isValid()) {
        return root(parent);
    }
    return child;
}

}
