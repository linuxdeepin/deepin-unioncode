// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLESCIPYTHON_H
#define STYLESCIPYTHON_H

#include "textedittabwidget/style/stylesci.h"
#include "common/common.h"

class StyleSciPython : public StyleSci
{
public:
    StyleSciPython(TextEdit *parent);
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle() override;
    virtual void setLexer() override;
    virtual int sectionEnd() const override;

protected:
    virtual void setThemeColor(DGuiApplicationHelper::ColorType colorType) override;
};

#endif // STYLESCIPYTHON_H
