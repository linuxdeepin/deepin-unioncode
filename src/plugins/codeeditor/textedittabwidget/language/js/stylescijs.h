// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLESCIJS_H
#define STYLESCIJS_H

#include "textedittabwidget/style/stylesci.h"
#include "common/common.h"

class StyleSciJS : public StyleSci
{
public:
    StyleSciJS(TextEdit *parent);
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle() override;
    virtual void setLexer() override;
    virtual int sectionEnd() const override;
    virtual int sectionStart() const override;
};

#endif // STYLESCIJS_H
