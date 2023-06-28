// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optiongradlegenerator.h"
#include "gradleoptionwidget.h"

#include "common/widget/pagewidget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QTabWidget>

class OptionGradleGeneratorPrivate
{
    friend class OptionGradleGenerator;

    GradleOptionWidget *widget = nullptr;
};

OptionGradleGenerator::OptionGradleGenerator()
    : d(new OptionGradleGeneratorPrivate())
{
    d->widget = new GradleOptionWidget();
}

QWidget *OptionGradleGenerator::optionWidget()
{
    return d->widget;
}
