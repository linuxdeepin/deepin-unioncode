// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditjs.h"
#include "stylelspjs.h"
#include "stylescijs.h"
#include "textedittabwidget/style/stylejsonfile.h"

class TextEditJSPrivate
{
    friend class TextEditJS;
    StyleLsp *styleLsp {nullptr};
    StyleSci *styleSci {nullptr};
    StyleJsonFile *styleFile {nullptr};
};

TextEditJS::TextEditJS(QWidget *parent)
    : TextEdit (parent)
    , d (new TextEditJSPrivate)
{
    d->styleFile = new StyleJsonFile(this);
    d->styleFile->setLanguage(this->supportLanguage());
    d->styleFile->setTheme(StyleJsonFile::Theme::get()->Dark);
    d->styleSci = new StyleSciJS(this);
    d->styleLsp = new StyleLspJS(this);
}

TextEditJS::~TextEditJS()
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

QString TextEditJS::supportLanguage()
{
    return "js";
}

QString TextEditJS::implLanguage()
{
    return "js";
}

StyleLsp *TextEditJS::getStyleLsp() const
{
    return d->styleLsp;
}

StyleSci *TextEditJS::getStyleSci() const
{
    return d->styleSci;
}

StyleJsonFile *TextEditJS::getStyleFile() const
{
    return d->styleFile;
}

