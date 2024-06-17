// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractdebugger.h"

// Constructor
AbstractDebugger::AbstractDebugger(QObject* parent) : QObject(parent)
{
    //Basic initialization, but more actions could be added here if needed
    // For example, connect signals from other objects to slots of this class,
    // initialize member variables, etc.
}

// Destructor
AbstractDebugger::~AbstractDebugger()
{
    // Release resources, disconnect signals, etc.
}
