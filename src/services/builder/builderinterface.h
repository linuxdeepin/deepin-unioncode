// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERINTERFACE_H
#define BUILDERINTERFACE_H

#include "builderglobals.h"
#include <framework/framework.h>

class BuilderInterface
{
public:
    /*!
     * \brief builderCommand
     * \param commandInfo
     */
    DPF_INTERFACE(void, builderCommand, const QList<BuildCommandInfo> &commandInfo, bool isSynchronous);
};

#endif // BUILDERINTERFACE_H
