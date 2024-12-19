// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optioncustommodelsgenerator.h"
#include "custommodelsoptionwidget.h"

#include "common/widget/pagewidget.h"

#include <DWidget>
#include <DTabWidget>

#include <QHBoxLayout>

OptionCustomModelsGenerator::OptionCustomModelsGenerator()
{

}

QWidget *OptionCustomModelsGenerator::optionWidget()
{
    return new CustomModelsOptionWidget();
}
