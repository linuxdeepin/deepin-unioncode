#ifndef STYLESCICMAKE_H
#define STYLESCICMAKE_H

#include "textedittabwidget/style/stylesci.h"

class StyleSciCmake : public StyleSci
{
public:
    StyleSciCmake(TextEdit *parent);
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle() override;
    virtual void setLexer() override;
    virtual int styleOffset() const override;
};

#endif // STYLESCICMAKE_H
