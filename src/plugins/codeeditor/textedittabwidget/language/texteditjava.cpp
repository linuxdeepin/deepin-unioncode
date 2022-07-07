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
    return "cpp"; // 向上兼容Scintilla中Lexer
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
