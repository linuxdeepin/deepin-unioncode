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
