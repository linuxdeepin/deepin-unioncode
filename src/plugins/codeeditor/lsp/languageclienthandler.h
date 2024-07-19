// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LANGUAGECLIENTHANDLER_H
#define LANGUAGECLIENTHANDLER_H

#include "common/common.h"

#include <QObject>

class TextEditor;
class LanguageClientHandlerPrivate;
class LanguageClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit LanguageClientHandler(TextEditor *parent);
    ~LanguageClientHandler();

    void requestCompletion(int line, int column);
    void updateTokens();
    void refreshTokens();
    lsp::SemanticTokenType::type_value tokenToDefine(int token);
    QColor symbolIndicColor(lsp::SemanticTokenType::type_value token,
                            QList<lsp::SemanticTokenType::type_index> modifier);

public slots:
    void switchHeaderSource(const QString &file);
    void followSymbolUnderCursor();
    void findUsagesActionTriggered();
    void renameActionTriggered();
    void formatSelections();

signals:
    void completeFinished(const lsp::CompletionProvider &provider);

private:
    QSharedPointer<LanguageClientHandlerPrivate> d { nullptr };
};

#endif   // LANGUAGECLIENTHANDLER_H
