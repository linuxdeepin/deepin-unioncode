// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
};

#endif // PROJECTVIEWINTERFACE_H
