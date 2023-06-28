// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditjava.h"
#include "stylelspjava.h"
#include "stylescijava.h"
#include "textedittabwidget/style/stylejsonfile.h"

class TextEditJavaPrivate
{
    friend class TextEditJava;
    StyleLsp *styleLsp {nullptr};
    StyleSci *styleSci {nullptr};
    StyleJsonFile *styleFile {nullptr};
};

TextEditJava::TextEditJava(QWidget *parent)
    : TextEdit (parent)
    , d (new TextEditJavaPrivate)
{
    d->styleFile = new StyleJsonFile(this);
    d->styleFile->setLanguage(this->supportLanguage());
    d->styleFile->setTheme(StyleJsonFile::Theme::get()->Dark);
    d->styleSci = new StyleSciJava(this);
    d->styleLsp = new StyleLspJava(this);
}

TextEditJava::~TextEditJava()
{
    if (d) {
        if (d->styleLsp)
            delete d->styleLsp;
        if (d->styleSci)
            delete d->styleSci;
        if (d->styleFile)
            delete d->styleFile;
    }
}

QString TextEditJava::supportLanguage()
{
    return "java"; // 向上兼容Scintilla中Lexer
}

QString TextEditJava::implLanguage()
{
    return "java"; // 界面调用兼容
}

StyleLsp *TextEditJava::getStyleLsp() const
{
    return d->styleLsp;
}

StyleSci *TextEditJava::getStyleSci() const
{
    return d->styleSci;
}

StyleJsonFile *TextEditJava::getStyleFile() const
{
    return d->styleFile;
}
