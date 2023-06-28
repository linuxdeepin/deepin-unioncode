// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITPYTHON_H
#define TEXTEDITPYTHON_H

#include "textedittabwidget/textedit.h"

class TextEditPythonPrivate;
class TextEditPython : public TextEdit
{
    Q_OBJECT
    TextEditPythonPrivate *const d;
public:
    explicit TextEditPython(QWidget * parent = nullptr);
    virtual ~TextEditPython();
    virtual QString supportLanguage();
    static QString implLanguage();

    virtual StyleLsp *getStyleLsp() const;
    virtual StyleSci *getStyleSci() const;
    virtual StyleJsonFile *getStyleFile() const;
};

#endif // TEXTEDITPYTHON_H
