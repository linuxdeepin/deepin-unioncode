// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLELSPJAVA_H
#define STYLELSPJAVA_H

#include "textedittabwidget/style/stylelsp.h"

class TextEdit;
class StyleLspJava : public StyleLsp
{
public:
    StyleLspJava(TextEdit *parent);
    virtual IndicStyleExt symbolIndic(lsp::SemanticTokenType::type_value token,
                                      QList<lsp::SemanticTokenModifier::type_index> modifier) override;
};

#endif // STYLELSPJAVA_H
