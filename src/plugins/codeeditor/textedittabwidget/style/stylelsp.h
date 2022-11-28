/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef STYLELSP_H
#define STYLELSP_H

#include <QObject>
#include <QList>

#include "stylesci.h"
#include "ScintillaEditBase.h"

#include "common/common.h"

#include <iostream>

class TextEdit;
class StyleLspPrivate;
class StyleLsp : public QObject
{
    Q_OBJECT
    StyleLspPrivate *const d;
public:

    enum Margin
    {
        LspCustom = StyleSci::Margin::Custom
    };

    enum MarkerNumber
    {
        Error = StyleSci::MarkerNumber::Extern,
        Warning,
        Information,
        Hint,
        ErrorLineBackground,
        WarningLineBackground,
        InformationLineBackground,
        HintLineBackground,
    };

    struct IndicStyleExt
    {
        QMap<int, int> fore;
    };

    enum EOLAnnotation
    {
        RedTextFore = 63,
    };

    static Sci_Position getSciPosition(sptr_t doc, const newlsp::Position &pos);
    static lsp::Position getLspPosition(sptr_t doc, sptr_t sciPosition);
    static int getLspCharacter(sptr_t doc, sptr_t sciPosition);
    static bool isCharSymbol(const char ch);

    StyleLsp(TextEdit *parent);
    TextEdit *edit();
    virtual ~StyleLsp();

    virtual void initLspConnection(); //setting main

    virtual IndicStyleExt symbolIndic(lsp::SemanticTokenType::type_value token,
                                      QList<lsp::SemanticTokenType::type_index> modifier);
    /*!
     * \brief tokenFromServProvider find local token from lsp protocol init response provider data
     * \param token from token request (tokenFull) key
     * \return local key
     */
    virtual lsp::SemanticTokenType::type_value tokenToDefine(int token);

    virtual void setIndicStyle();
    virtual void setMargin();

    virtual void setDiagnostics(const newlsp::PublishDiagnosticsParams &data);
    virtual void cleanDiagnostics();

    virtual void setTokenFull(const QList<lsp::Data> &tokens);
    virtual void cleanTokenFull();

    virtual void setHover(const newlsp::Hover &hover);
    virtual void cleanHover();

    virtual void setDefinition(const newlsp::Location &data);
    virtual void setDefinition(const std::vector<newlsp::Location> &data);
    virtual void setDefinition(const std::vector<newlsp::LocationLink> &data);
    virtual void cleanDefinition(const Scintilla::Position &pos);

private slots:
    void setDefinitionSelectedStyle(const Scintilla::Position start,
                                    const Scintilla::Position end);
    void setCompletion(const QByteArray &text,
                       const Scintilla::Position enterLenght,
                       const lsp::CompletionProvider &provider);

    void sciTextInsertedTotal(Scintilla::Position position,
                              Scintilla::Position length, Scintilla::Position linesAdded,
                              const QByteArray &text, Scintilla::Position line);

    void sciTextDeletedTotal(Scintilla::Position position,
                             Scintilla::Position length, Scintilla::Position linesAdded,
                             const QByteArray &text, Scintilla::Position line);

    void sciLinesAdded(Scintilla::Position position);
    void SciCharAdded(int ch);
    void sciTextChangedTotal();
    void sciHovered(Scintilla::Position position);
    void sciHoverCleaned(Scintilla::Position position);
    void sciDefinitionHover(Scintilla::Position position);
    void sciDefinitionHoverCleaned(Scintilla::Position position);
    void sciIndicClicked(Scintilla::Position position);
    void sciIndicReleased(Scintilla::Position position);
    void sciSelectionMenu(QContextMenuEvent *event);
    void sciReplaced(const QString &file, Scintilla::Position start,
                     Scintilla::Position end, const QString &text);
    void renameRequest(const QString &newText);
};

#endif // STYLELSP_H
