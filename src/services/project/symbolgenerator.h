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
#ifndef SYMBOLGENERATOR_H
#define SYMBOLGENERATOR_H

#include "projectinfo.h"
#include "common/common.h"

namespace dpfservice {

/*!
 * \brief The SymbolGenerator class 符号生成器类
 */
class SymbolGenerator : public Generator
{
    Q_OBJECT
public:

    SymbolGenerator(){}

    /*!
     * \brief createRootItem 创建文件树路径，子类需要重载实现
     *  执行该函数应当首先确定前置条件的满足，比如已经执行了生成器的过程。
     * \param projectPath 工程文件路径
     * \return
     */
    virtual QStandardItem *createRootItem(const ProjectInfo &info) {
        Q_UNUSED(info);
        return nullptr;
    }
};


} // namespace dpfservice
#endif // SYMBOLGENERATOR_H
