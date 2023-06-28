// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionpythongenerator.h"
#include "pythonoptionwidget.h"

#include <QWidget>
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

QWidget *OptionPythonGenerator::optionWidget()
{
    return d->widget;
}
