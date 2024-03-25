// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionninjagenerator.h"
#include "ninjaoptionwidget.h"

#include "common/widget/pagewidget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QTabWidget>

class OptionNinjaGeneratorPrivate
{
    friend class OptionNinjaGenerator;

    NinjaOptionWidget *widget = nullptr;
};

OptionNinjaGenerator::OptionNinjaGenerator()
    : d(new OptionNinjaGeneratorPrivate())
{
    d->widget = new NinjaOptionWidget();
}

OptionNinjaGenerator::~OptionNinjaGenerator()
{
    delete d;
}

QWidget *OptionNinjaGenerator::optionWidget()
{
    return d->widget;
}
