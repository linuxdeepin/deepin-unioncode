// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
