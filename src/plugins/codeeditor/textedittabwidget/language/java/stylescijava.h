// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLESCIJAVA_H
#define STYLESCIJAVA_H

#include "textedittabwidget/style/stylesci.h"

class StyleSciJava : public StyleSci
{
public:
    StyleSciJava(TextEdit *parent);
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle() override;
    virtual void setLexer() override;
    virtual int sectionEnd() const override;
};

#endif // STYLESCIJAVA_H
