// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optiondefaultkeeper.h"

OptionDefaultKeeper::OptionDefaultKeeper()
{

}

OptionDefaultKeeper::~OptionDefaultKeeper()
{
    if (optionDialog)
        delete optionDialog;
}

OptionDefaultKeeper *OptionDefaultKeeper::instance()
{
    if (!ins)
        ins = new OptionDefaultKeeper();
    return ins;
}

OptionsDialog *OptionDefaultKeeper::getOptionDialog()
{
    if (!optionDialog) {
        optionDialog = new OptionsDialog();
    }
    return optionDialog;
}
