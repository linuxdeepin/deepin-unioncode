// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
