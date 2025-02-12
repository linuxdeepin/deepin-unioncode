// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionchatgenerator.h"
#include "chatoptionwidget.h"

#include "common/widget/pagewidget.h"

#include <DWidget>
#include <DTabWidget>

#include <QHBoxLayout>

OptionChatGenerator::OptionChatGenerator()
{

}

QWidget *OptionChatGenerator::optionWidget()
{
    return new ChatOptionWidget();
}
