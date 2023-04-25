/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

