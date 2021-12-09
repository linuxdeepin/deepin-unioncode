/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef GLOBAL_DFM_FRAMEWORK_H
#define GLOBAL_DFM_FRAMEWORK_H

#define DPF_BEGIN_NAMESPACE namespace dpf{
#define DPF_END_NAMESPACE }
#define DPF_USE_NAMESPACE using namespace dpf;
#define DPF_NAMESPACE dpf

#if defined(DPF_LIBRARY)
#  define DPF_EXPORT Q_DECL_EXPORT
#else
#  define DPF_EXPORT Q_DECL_IMPORT
#endif


#endif // GLOBAL_DFM_FRAMEWORK_H
