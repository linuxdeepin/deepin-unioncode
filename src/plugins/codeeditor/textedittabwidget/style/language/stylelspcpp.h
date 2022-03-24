#ifndef STYLELSPCPP_H
#define STYLELSPCPP_H

#include "textedittabwidget/style/stylelsp.h"

class StyleLspCpp : public StyleLsp
{
    Q_OBJECT
public:
    StyleLspCpp();
    virtual ServerInfo clientInfoSpec(ServerInfo info) override;
    virtual IndicStyleExt symbolIndic(ScintillaEdit &edit,
                                      lsp::SemanticTokenType::type_value token,
                                      QList<lsp::SemanticTokenModifier::type_index> modifier) override;
};

#endif // STYLELSPCPP_H
