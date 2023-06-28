// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texteditpython.h"
#include "stylelsppython.h"
#include "stylescipython.h"
#include "textedittabwidget/style/stylejsonfile.h"

class TextEditPythonPrivate
{
    friend class TextEditPython;
    StyleLsp *styleLsp {nullptr};
    StyleSci *styleSci {nullptr};
    StyleJsonFile *styleFile {nullptr};
};

TextEditPython::TextEditPython(QWidget *parent)
    : TextEdit (parent)
    , d (new TextEditPythonPrivate)
{
    d->styleFile = new StyleJsonFile(this);
    d->styleFile->setLanguage(this->supportLanguage());
    d->styleFile->setTheme(StyleJsonFile::Theme::get()->Dark);
    d->styleSci = new StyleSciPython(this);
    d->styleLsp = new StyleLspPython(this);
}

TextEditPython::~TextEditPython()
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

QString TextEditPython::supportLanguage()
{
    return "python"; // 向上兼容Scintilla中Lexer
}

QString TextEditPython::implLanguage()
{
    return "python"; // 界面调用兼容
}

StyleLsp *TextEditPython::getStyleLsp() const
{
    return d->styleLsp;
}

StyleSci *TextEditPython::getStyleSci() const
{
    return d->styleSci;
}

StyleJsonFile *TextEditPython::getStyleFile() const
{
    return d->styleFile;
}

