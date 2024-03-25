// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionpythongenerator.h"
#include "pythonoptionwidget.h"

#include <DWidget>
#include <QHBoxLayout>
#include <QTabWidget>

class OptionPythonGeneratorPrivate
{
    friend class OptionPythonGenerator;

    PythonOptionWidget *widget = nullptr;
};

OptionPythonGenerator::OptionPythonGenerator()
    : d(new OptionPythonGeneratorPrivate())
{
    d->widget = new PythonOptionWidget();
}

OptionPythonGenerator::~OptionPythonGenerator()
{
    delete d;
}

DWidget *OptionPythonGenerator::optionWidget()
{
    return d->widget;
}
