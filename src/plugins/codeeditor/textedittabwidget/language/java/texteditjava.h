// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITJAVA_H
#define TEXTEDITJAVA_H

#include "textedittabwidget/textedit.h"

class TextEditJavaPrivate;
class TextEditJava : public TextEdit
{
    Q_OBJECT
    TextEditJavaPrivate *const d;
public:
    explicit TextEditJava(QWidget * parent = nullptr);
    virtual ~TextEditJava();
    virtual QString supportLanguage();
    static QString implLanguage();

    virtual StyleLsp *getStyleLsp() const;
    virtual StyleSci *getStyleSci() const;
    virtual StyleJsonFile *getStyleFile() const;
};

#endif // TEXTEDITJAVA_H
