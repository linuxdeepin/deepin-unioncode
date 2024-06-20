// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorutils.h"
#include "common/common.h"

int EditorUtils::nbDigitsFromNbLines(long nbLines)
{
    int nbDigits = 0;   // minimum number of digit should be 4
    if (nbLines < 10)
        nbDigits = 1;
    else if (nbLines < 100)
        nbDigits = 2;
    else if (nbLines < 1000)
        nbDigits = 3;
    else if (nbLines < 10000)
        nbDigits = 4;
    else if (nbLines < 100000)
        nbDigits = 5;
    else if (nbLines < 1000000)
        nbDigits = 6;
    else {   // rare case
        nbDigits = 7;
        nbLines /= 1000000;

        while (nbLines) {
            nbLines /= 10;
            ++nbDigits;
        }
    }

    return nbDigits;
}
