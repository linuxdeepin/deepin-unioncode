// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITJS_H
#define TEXTEDITJS_H

#include "textedittabwidget/textedit.h"

class TextEditJSPrivate;
class TextEditJS : public TextEdit
{
    Q_OBJECT
    TextEditJSPrivate *const d;
public:
    explicit TextEditJS(QWidget * parent = nullptr);
    virtual ~TextEditJS();
    virtual QString supportLanguage();
    static QString implLanguage();

    virtual StyleLsp *getStyleLsp() const;
    virtual StyleSci *getStyleSci() const;
    virtual StyleJsonFile *getStyleFile() const;
};

#endif // TEXTEDITJS_H
