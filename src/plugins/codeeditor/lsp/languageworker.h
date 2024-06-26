// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LANGUAGEWORKER_H
#define LANGUAGEWORKER_H

#include "languageclienthandler.h"

#include "common/lsp/protocol/protocol.h"

#include <QObject>

class TextEditor;
class LanguageWorker : public QObject
{
    Q_OBJECT
public:
    struct DocumentToken
    {
        int startPostion = -1;
        QString field;
        QColor color;
    };

    explicit LanguageWorker(LanguageClientHandler *handler);

    void stop();
    void setTextEditor(TextEditor *edit);

public Q_SLOTS:
    void handleDocumentSemanticTokens(const QList<lsp::Data> &tokens);

Q_SIGNALS:
    void highlightToken(const QList<DocumentToken> &tokenList);

private:
    LanguageClientHandler *clientHandler { nullptr };
    TextEditor *textEditor { nullptr };
    QAtomicInteger<bool> isStop {false};
};

Q_DECLARE_METATYPE(LanguageWorker::DocumentToken)
#endif   // LANGUAGEWORKER_H
