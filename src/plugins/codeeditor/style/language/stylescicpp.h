#ifndef STYLESCICPP_H
#define STYLESCICPP_H

#include "../stylesci.h"

class StyleSciCpp : public StyleSci
{
public:
    StyleSciCpp();
    virtual const char *sciKeyWords(int set);
    virtual void setDefaultStyle();
    virtual int indicOffset();
};

#endif // STYLESCICPP_H
