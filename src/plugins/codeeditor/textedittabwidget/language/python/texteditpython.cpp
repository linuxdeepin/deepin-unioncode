/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

