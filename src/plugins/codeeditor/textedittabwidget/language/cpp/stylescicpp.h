// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLESCICPP_H
#define STYLESCICPP_H

#include "textedittabwidget/style/stylesci.h"

class StyleSciCpp : public StyleSci
{
public:
    StyleSciCpp(TextEdit *parent);
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle() override;
    virtual int sectionEnd() const override;
};

#endif // STYLESCICPP_H
