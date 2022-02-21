#ifndef STYLESCICMAKE_H
#define STYLESCICMAKE_H

#include "textedittabwidget/style/stylesci.h"

class StyleSciCmake : public StyleSci
{
public:
    StyleSciCmake();
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle(ScintillaEdit &edit) override;
    virtual void setLexer(ScintillaEdit &edit) override;
    virtual int styleOffset() const override;
};

#endif // STYLESCICMAKE_H
