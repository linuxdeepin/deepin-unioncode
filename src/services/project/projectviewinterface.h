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
#ifndef PROJECTVIEWINTERFACE_H
#define PROJECTVIEWINTERFACE_H

#include "projectinfo.h"

#include <framework/framework.h>

class QStandardItem;
class ProjectViewInterface
{
public:
    /*!
     * \brief addRootItem 添加工程根数据节点
     * \param aitem
     */
    DPF_INTERFACE(void, addRootItem, QStandardItem *aitem);

    /*!
     * \brief removeRootItem 删除工程根数据节点
     * \param aitem
     */
    DPF_INTERFACE(void, removeRootItem, QStandardItem *aitem);

    /*!
     * \brief takeRootItem 从View移除但不删除工程节点
     * \param aitem
     */
    DPF_INTERFACE(void, takeRootItem, QStandardItem *aitem);

    /*!
     * \brief expandedDepth 展开工程子项根据深度
     * \param aitem root节点
     * \param depth 深度
     */
    DPF_INTERFACE(void, expandedDepth, QStandardItem *aitem, int depth);

    /*!
     * \brief expandedAll 展开工程所有子项
     * \param aitem root节点
     */
    DPF_INTERFACE(void, expandedAll, QStandardItem *aitem);


    /*!
     * \brief getAllProjectInfo 获取所有工程信息
     */
    DPF_INTERFACE(QList<dpfservice::ProjectInfo>, getAllProjectInfo, void);
};

#endif // PROJECTVIEWINTERFACE_H
