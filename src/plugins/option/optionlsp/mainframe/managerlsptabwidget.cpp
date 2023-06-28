// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "managerlsptabwidget.h"
#include "adapterconfigure.h"
#include "services/window/windowelement.h"

ManagerLspTabWidget::ManagerLspTabWidget(QWidget *parent)
    : QTabWidget (parent)
{
    using namespace dpfservice;
    addTab(new AdapterConfigure, MWMFA_CXX);
    addTab(new AdapterConfigure, MWMFA_JAVA);
    addTab(new AdapterConfigure, MWMFA_PYTHON);
}
