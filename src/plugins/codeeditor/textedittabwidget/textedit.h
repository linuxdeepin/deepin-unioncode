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
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include "scintillaeditextern.h"

class StyleLsp;
class StyleSci;
class StyleJsonFile;
class TextEditPrivate;
class TextEdit : public ScintillaEditExtern
{
    Q_OBJECT
    TextEditPrivate *const d;
public:
    explicit TextEdit(QWidget * parent = nullptr);
    virtual ~TextEdit();
    virtual void setFile(const QString &filePath) override;
    virtual StyleLsp *getStyleLsp() const {return nullptr;}
    virtual StyleSci *getStyleSci() const {return nullptr;}
    virtual StyleJsonFile *getStyleFile() const {return nullptr;}
};

#endif // TEXTEDIT_H
