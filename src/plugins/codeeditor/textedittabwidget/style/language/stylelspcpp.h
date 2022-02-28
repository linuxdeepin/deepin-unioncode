#ifndef STYLELSPCPP_H
#define STYLELSPCPP_H

#include "textedittabwidget/style/stylelsp.h"

class StyleLspCpp : public StyleLsp
{
    Q_OBJECT
public:
    StyleLspCpp();
    ServerInfo clientInfoSpec(ServerInfo info) override;
};

#endif // STYLELSPCPP_H
