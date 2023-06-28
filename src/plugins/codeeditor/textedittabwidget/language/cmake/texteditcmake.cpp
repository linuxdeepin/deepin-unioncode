// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditcmake.h"
#include "stylelspcmake.h"
#include "stylescicmake.h"
#include "textedittabwidget/style/stylejsonfile.h"

class TextEditCmakePrivate
{
    friend class TextEditCmake;
    StyleLsp *styleLsp {nullptr};
    StyleSci *styleSci {nullptr};
    StyleJsonFile *styleFile {nullptr};
};

TextEditCmake::TextEditCmake(QWidget *parent)
    : TextEdit (parent)
    , d (new TextEditCmakePrivate)
{
    d->styleFile = new StyleJsonFile(this);
    d->styleFile->setLanguage(this->supportLanguage());
    d->styleFile->setTheme(StyleJsonFile::Theme::get()->Dark);
    d->styleSci = new StyleSciCmake(this);
//    d->styleLsp = new StyleLspCmake(this);
}

TextEditCmake::~TextEditCmake()
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

QString TextEditCmake::supportLanguage()
{
    return "cmake"; // 向上兼容Scintilla中Lexer
}

QString TextEditCmake::implLanguage()
{
    return "cmake"; // 界面调用兼容
}

StyleLsp *TextEditCmake::getStyleLsp() const
{
    return d->styleLsp;
}

StyleSci *TextEditCmake::getStyleSci() const
{
    return d->styleSci;
}

StyleJsonFile *TextEditCmake::getStyleFile() const
{
    return d->styleFile;
}

