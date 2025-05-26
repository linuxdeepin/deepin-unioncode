// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionjavagenerator.h"
#include "javaoptionwidget.h"

#include "common/widget/pagewidget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QTabWidget>

OptionJavaGenerator::OptionJavaGenerator()
{

}

QWidget *OptionJavaGenerator::optionWidget()
{
    return new JavaOptionWidget();
}
