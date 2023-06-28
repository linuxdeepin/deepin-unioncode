// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLELSPCPP_H
#define STYLELSPCPP_H

#include "textedittabwidget/style/stylelsp.h"

class StyleLspCpp : public StyleLsp
{
    Q_OBJECT
public:
    StyleLspCpp(TextEdit *parent);
    virtual IndicStyleExt symbolIndic(lsp::SemanticTokenType::type_value token,
                                      QList<lsp::SemanticTokenModifier::type_index> modifier) override;
};

#endif // STYLELSPCPP_H
