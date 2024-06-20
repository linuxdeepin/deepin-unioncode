// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LSPSTYLE_H
#define LSPSTYLE_H

#include "common/common.h"

#include <QObject>

class TextEditor;
class LSPStylePrivate;
class LSPStyle : public QObject
{
    Q_OBJECT
public:
    explicit LSPStyle(TextEditor *parent);
    virtual ~LSPStyle();

    virtual void initLspConnection();   //setting main
    void updateTokens();
    void setIndicStyle();
    virtual QColor symbolIndicColor(lsp::SemanticTokenType::type_value token,
                                    QList<lsp::SemanticTokenType::type_index> modifier);

    /*!
     * \brief tokenFromServProvider find local token from lsp protocol init response provider data
     * \param token from token request (tokenFull) key
     * \return local key
     */
    virtual lsp::SemanticTokenType::type_value tokenToDefine(int token);

    virtual void setMargin();

    virtual void setDiagnostics(const newlsp::PublishDiagnosticsParams &data);
    virtual void cleanDiagnostics();

    virtual void setTokenFull(const QList<lsp::Data> &tokens);
    virtual void refreshTokens();
    virtual void cleanTokenFull();

    virtual void setHover(const newlsp::Hover &hover);

    virtual void setDefinition(const newlsp::Location &data);
    virtual void setDefinition(const std::vector<newlsp::Location> &data);
    virtual void setDefinition(const std::vector<newlsp::LocationLink> &data);
    virtual void cleanDefinition(int pos);

    virtual void rangeFormattingReplace(const std::vector<newlsp::TextEdit> &edits);

private slots:
    void setDefinitionSelectedStyle(int start, int end);
    void setCompletion(const QString &text, int enterLenght,
                       const lsp::CompletionProvider &provider);

    void onTextInsertedTotal(int position, int length, int linesAdded, const QString &text, int line);
    void onTextDeletedTotal(int position, int length, int linesAdded, const QString &text, int line);
    void onTextChangedTotal();
    void onHovered(int position);
    void onHoverCleaned(int position);
    void onDefinitionHover(int position);
    void onDefinitionHoverCleaned(int position);
    void onIndicClicked(int line, int index);
    void onShowContextMenu(QMenu *menu);
    void onFileClosed(const QString &file);
    void switchDeclarationOrDefinition();
    void findUsagesActionTriggered();
    void renameActionTriggered();
    void renameSymbol(const QString &text);
    void gotoDefinition();

private:
    QSharedPointer<LSPStylePrivate> d { nullptr };
};

#endif   // LSPSTYLE_H
