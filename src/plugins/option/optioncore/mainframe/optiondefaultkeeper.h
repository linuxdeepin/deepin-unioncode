// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONDEFAULTKEEPER_H
#define OPTIONDEFAULTKEEPER_H

#include "optionsdialog.h"

class OptionDefaultKeeper final
{
    OptionDefaultKeeper();
    OptionDefaultKeeper(const OptionDefaultKeeper &other) = delete;
    inline static OptionDefaultKeeper *ins {nullptr};
    inline static OptionsDialog *optionDialog {nullptr};
public:
    virtual ~OptionDefaultKeeper();
    static OptionDefaultKeeper *instance();
    static OptionsDialog* getOptionDialog();
};

#endif // OPTIONDEFAULTKEEPER_H
