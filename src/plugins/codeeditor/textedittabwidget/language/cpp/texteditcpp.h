// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITCPP_H
#define TEXTEDITCPP_H

#include "textedittabwidget/textedit.h"

class TextEditCppPrivate;
class TextEditCpp : public TextEdit
{
    Q_OBJECT
    TextEditCppPrivate *const d;
public:
    explicit TextEditCpp(QWidget * parent = nullptr);
    virtual ~TextEditCpp();
    virtual QString supportLanguage();
    static QString implLanguage();

    virtual StyleLsp *getStyleLsp() const;
    virtual StyleSci *getStyleSci() const;
    virtual StyleJsonFile *getStyleFile() const;
};

#endif // TEXTEDITCPP_H
