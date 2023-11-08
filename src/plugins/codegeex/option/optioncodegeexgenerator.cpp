// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optioncodegeexgenerator.h"
#include "codegeexoptionwidget.h"

#include "common/widget/pagewidget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QTabWidget>

OptionCodeGeeXGenerator::OptionCodeGeeXGenerator()
{

}

QWidget *OptionCodeGeeXGenerator::optionWidget()
{
    return new CodeGeeXOptionWidget();
}
