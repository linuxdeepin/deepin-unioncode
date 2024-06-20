// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionmavengenerator.h"
#include "mavenoptionwidget.h"

#include "common/widget/pagewidget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QTabWidget>

class OptionMavenGeneratorPrivate
{
    friend class OptionMavenGenerator;

    MavenOptionWidget *widget = nullptr;
};

OptionMavenGenerator::OptionMavenGenerator()
    : d(new OptionMavenGeneratorPrivate())
{
    d->widget = new MavenOptionWidget();
}

QWidget *OptionMavenGenerator::optionWidget()
{
    return d->widget;
}
