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

#ifndef PROJECTSERVICE_H
#define PROJECTSERVICE_H

#include "common/common.h"

#include <framework/framework.h>

#include <QStyledItemDelegate>
#include <QStandardItem>
#include <QMenu>

namespace dpfservice {

struct ProjectGenerator : public Generator
{
    Q_OBJECT
public:
    enum ItemDataRole
    {
        ToolKitProperty = Qt::ItemDataRole::UserRole,
    };

    enum_def(ToolKitPropertyKey, QString)
    {
        enum_exp ToolKitName = "ToolKitName";
    };

    ProjectGenerator(){}

    /*!
     * \brief createRootItem 創建文件樹數據
     * \param projectPath 工程文件路徑
     * \return
     */
    virtual QStandardItem *createRootItem(const QString &projectPath) {
        Q_UNUSED(projectPath);
        return nullptr;
    }

    /*!
     * \brief createIndexMenu 文件樹index點擊時觸發
     * \param index 索引數據
     * \return
     */
    virtual QMenu* createIndexMenu(const QModelIndex &index){
        Q_UNUSED(index);
        return nullptr;
    }

protected:
    /*!
     * \brief setToolKitName設置工具套件名稱, createRootItem實現時方便函數
     * \param item 文件樹節點
     * \param toolkit 名稱
     */
    static void setToolKitName(QStandardItem *item, const QString &name);

    /*!
     * \brief toolKitName 获取工具套件名称
     * \param item 文件树节点
     */
    static QString toolKitName(QStandardItem *item);

    /*!
     * \brief setToolKitProperty設置工具套件特性, createRootItem實現時方便函數
     * \param item 文件樹節點
     * \param key 鍵
     * \param value 值
     */
    static void setToolKitProperty(QStandardItem *item, const QString &key, const QVariant &value);

    /*!
     * \brief toolKitProperty 获取共计套件特性
     * \param item 文件树节点
     * \param key 键
     * \return 万能类型，需要知道存入方类型进行转换后使用
     */
    static QVariant toolKitProperty(QStandardItem *item, const QString &key);

    /*!
     * \brief toolKiyPropertyMap 获取所有特性值
     * \param item 文件树节点
     * \return 存入所有的特性Map
     */
    static QVariantMap toolKitPropertyMap(QStandardItem *item);
};

class ProjectService final : public dpf::PluginService,
        dpf::AutoServiceRegister<ProjectService>,
        dpf::QtClassFactory<ProjectGenerator>,
        dpf::QtClassManager<ProjectGenerator>

{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectService)
    typedef dpf::QtClassManager<ProjectGenerator> GeneratorManager;
    typedef dpf::QtClassFactory<ProjectGenerator> GeneratorFactory;
public:
    static QString name();
    explicit ProjectService(QObject *parent = nullptr);

    /*!
     * \brief supportGenerator 獲取支持的工程名稱
     * \return
     */
    QStringList supportGeneratorName()
    {
        return GeneratorFactory::createKeys();
    }

    /*!
     * \brief implGenerator 導入工程文件生成器對象 T = Project::Generator類泛化對象
     * \param name 生成器對象名稱(唯一鍵)
     * \param errorString 錯誤信息
     */
    template<class T = ProjectGenerator>
    bool implGenerator(const QString &name, QString *errorString = nullptr)
    {
        return GeneratorFactory::regClass<T>(name, errorString);
    }

    /*!
     * \brief createGenerator
     * \param name 名稱
     * \param errorString 錯誤信息
     * \return 生成器對象實例
     */
    ProjectGenerator *createGenerator(const QString &name, QString *errorString = nullptr)
    {
        auto generator = GeneratorManager::value(name);
        if (!generator) {
            generator = GeneratorFactory::create(name, errorString);
            if (generator)
                GeneratorManager::append(name, generator);
        }
        return generator;
    }

    /*!
     * \brief addProjectRootItem 添加根數據到TreeView
     * \param aitem
     */
    DPF_INTERFACE(void, addProjectRootItem, QStandardItem *aitem);

    /*!
     * \brief DPF_INTERFACE
     * \param level
     */
    DPF_INTERFACE(void, expandedProjectDepth, QStandardItem *aitem, int depth);

    DPF_INTERFACE(void, expandedProjectAll, QStandardItem *aitem);
};
} //namespace dpfservice

#endif // PROJECTSERVICE_H
