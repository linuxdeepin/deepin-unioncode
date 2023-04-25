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
#ifndef STYLESCI_H
#define STYLESCI_H

#include "common/common.h"

#include <QObject>
#include <QColor>

class TextEdit;
class StyleSciPrivate;
class StyleSci : public QObject
{
    Q_OBJECT
    StyleSciPrivate *const d;
public:
    enum Margin
    {
        LineNumber = 0,
        Runtime = 1,
        CodeFormat = 2,
        Custom = 3,
    };

    enum MarkerNumber
    {
        Debug = 0,
        Running = 1,
        RunningLineBackground = 2,
        CustomLineBackground = 3,
        Extern = 4
    };

    StyleSci(TextEdit *parent);
    virtual ~StyleSci();

    TextEdit *edit();
    void setKeyWords();
    virtual void setStyle();
    virtual void setMargin();
    virtual void setLexer();

    virtual QMap<int, QString> keyWords() const;
    virtual int sectionEnd() const;
    virtual int sectionStart() const;
};



#endif // STYLESCI_H
