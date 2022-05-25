#ifndef STYLESCICPP_H
#define STYLESCICPP_H

#include "textedittabwidget/style/stylesci.h"

class StyleSciCpp : public StyleSci
{
public:
    StyleSciCpp(TextEdit *parent);
    virtual QMap<int, QString> keyWords() const override;
    virtual void setStyle() override;
    virtual int styleOffset() const override;
};

#endif // STYLESCICPP_H
