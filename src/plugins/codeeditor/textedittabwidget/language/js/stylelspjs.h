// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLELSPJS_H
#define STYLELSPJS_H

#include "textedittabwidget/style/stylelsp.h"

class TextEdit;
class StyleLspJS : public StyleLsp
{
public:
    StyleLspJS(TextEdit *parent);
    virtual IndicStyleExt symbolIndic(lsp::SemanticTokenType::type_value token,
                                      QList<lsp::SemanticTokenModifier::type_index> modifier) override;
};

#endif // STYLELSPJS_H
