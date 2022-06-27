/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
 *             huangyu<huangyub@unioncode.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
