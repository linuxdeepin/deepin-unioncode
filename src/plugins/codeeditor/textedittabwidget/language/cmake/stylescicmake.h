// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLESCICMAKE_H
#define STYLESCICMAKE_H

#include "textedittabwidget/style/stylesci.h"

class StyleSciCmake : public StyleSci
{
public:
    StyleSciCmake(TextEdit *parent);
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle() override;
    virtual void setLexer() override;
    virtual int sectionEnd() const override;
};

#endif // STYLESCICMAKE_H
