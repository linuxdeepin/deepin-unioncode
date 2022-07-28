/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#ifndef OPTIONCMAKEGENERATOR_H
#define OPTIONCMAKEGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionCmakeGeneratorPrivate;
class OptionCmakeGenerator : public dpfservice::OptionGenerator
{
public:
    OptionCmakeGenerator();
    inline static QString kitName() {return "cmake";}
    virtual QWidget *optionWidget() override;

private:
    OptionCmakeGeneratorPrivate *const d;
};

#endif // OPTIONCMAKEGENERATOR_H
