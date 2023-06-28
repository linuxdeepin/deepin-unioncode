// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLELSPPYTHON_H
#define STYLELSPPYTHON_H

#include "textedittabwidget/style/stylelsp.h"

class TextEdit;
class StyleLspPython : public StyleLsp
{
public:
    StyleLspPython(TextEdit *parent);
    virtual IndicStyleExt symbolIndic(lsp::SemanticTokenType::type_value token,
                                      QList<lsp::SemanticTokenModifier::type_index> modifier) override;
};

#endif // STYLELSPPYTHON_H
