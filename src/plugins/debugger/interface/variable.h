// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VARIABLE_H
#define VARIABLE_H

#include "dap/protocol.h"

#include <stdlib.h>
#include <vector>
#include <string>
#include <QVector>

struct IVariable;
using IVariables = QVector<IVariable>;
struct IVariable
{
    dap::string name;
    dap::Variable var;
    dap::integer depth = 0;
    IVariables children;
};

#endif // VARIABLE_H
