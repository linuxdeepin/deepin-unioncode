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
    ~LSPStyle();

    void initLspConnection();   //setting main
    void requestCompletion(int line, int column);
    void updateTokens();
    void setIndicStyle();
    QList<newlsp::DocumentSymbol> documentSymbolList() const;
    QList<newlsp::SymbolInformation> symbolInformationList() const;
    QColor symbolIndicColor(lsp::SemanticTokenType::type_value token,
                            QList<lsp::SemanticTokenType::type_index> modifier);

    /*!
     * \brief tokenFromServProvider find local token from lsp protocol init response provider data
     * \param token from token request (tokenFull) key
     * \return local key
     */
    lsp::SemanticTokenType::type_value tokenToDefine(int token);

    void setDiagnostics(const newlsp::PublishDiagnosticsParams &data);
    void cleanDiagnostics();

    void setTokenFull(const QList<lsp::Data> &tokens, const QString &filePath);
    void refreshTokens();
    void cleanTokenFull();

    void setHover(const newlsp::Hover &hover);

    void setDefinition(const newlsp::Location &data, const QString &filePath);
    void setDefinition(const std::vector<newlsp::Location> &data, const QString &filePath);
    void setDefinition(const std::vector<newlsp::LocationLink> &data, const QString &filePath);
    void cleanDefinition(int pos);

    void rangeFormattingReplace(const std::vector<newlsp::TextEdit> &edits, const QString &filePath);

public slots:
    void switchHeaderSource(const QString &file);
    void followSymbolUnderCursor();
    void findUsagesActionTriggered();
    void renameActionTriggered();
    void formatSelections();

signals:
    void completeFinished(const lsp::CompletionProvider &provider);

private slots:
    void setDefinitionSelectedStyle(int start, int end);

    void onTextChanged();
    void onHovered(int position);
    void onHoverCleaned(int position);
    void onDefinitionHover(int position);
    void onDefinitionHoverCleaned();
    void onIndicClicked(int line, int index);
    void onShowContextMenu(QMenu *menu);
    void onFileClosed(const QString &file);
    void renameSymbol(const QString &text);
    void gotoDefinition();
    void handleSwitchHeaderSource(const QString &file);
    void handleDocumentSymbolResult(const QList<newlsp::DocumentSymbol> &docSymbols, const QString &filePath);
    void handleSymbolInfomationResult(const QList<newlsp::SymbolInformation> &symbolInfos, const QString &filePath);

private:
    QSharedPointer<LSPStylePrivate> d { nullptr };
};

#endif   // LSPSTYLE_H
