// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditcpp.h"
#include "stylelspcpp.h"
#include "stylescicpp.h"
#include "textedittabwidget/style/stylejsonfile.h"

class TextEditCppPrivate
{
    friend class TextEditCpp;
    StyleLsp *styleLsp {nullptr};
    StyleSci *styleSci {nullptr};
    StyleJsonFile *styleFile {nullptr};
};

TextEditCpp::TextEditCpp(QWidget *parent)
    : TextEdit (parent)
    , d (new TextEditCppPrivate)
{
    d->styleFile = new StyleJsonFile(this);
    d->styleFile->setLanguage(this->supportLanguage());
    d->styleFile->setTheme(StyleJsonFile::Theme::get()->Dark);
    d->styleSci = new StyleSciCpp(this);
    d->styleLsp = new StyleLspCpp(this);
}

TextEditCpp::~TextEditCpp()
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

QString TextEditCpp::supportLanguage()
{
    return "cpp"; // 向上兼容Scintilla中Lexer
}

QString TextEditCpp::implLanguage()
{
    return "cpp"; // 界面调用兼容
}

StyleLsp *TextEditCpp::getStyleLsp() const
{
    return d->styleLsp;
}

StyleSci *TextEditCpp::getStyleSci() const
{
    return d->styleSci;
}

StyleJsonFile *TextEditCpp::getStyleFile() const
{
    return d->styleFile;
}
