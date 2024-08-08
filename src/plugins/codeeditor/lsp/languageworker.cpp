// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "languageworker.h"
#include "gui/texteditor.h"

LanguageWorker::LanguageWorker(LanguageClientHandler *handler)
    : clientHandler(handler)
{
    qRegisterMetaType<QList<DocumentToken>>("QList<DocumentToken>");
}

void LanguageWorker::stop()
{
    isStop = true;
}

void LanguageWorker::setTextEditor(TextEditor *edit)
{
    textEditor = edit;
}

void LanguageWorker::handleDocumentSemanticTokens(const QList<lsp::Data> &tokens)
{
    isStop = false;
    QList<DocumentToken> docTokenList;
    int cacheLine = 0;
    int cacheColumn = 0;
    for (auto val : tokens) {
        if (isStop)
            return;

        cacheLine += val.start.line;
        if (val.start.line != 0)
            cacheColumn = 0;

        cacheColumn += val.start.character;
        auto startPos = textEditor->positionFromLineIndex(cacheLine, cacheColumn);
        QString sourceText = textEditor->text(startPos, startPos + val.length);
        if (!sourceText.isEmpty()) {
            QString tokenValue = clientHandler->tokenToDefine(val.tokenType);
            QColor color = clientHandler->symbolIndicColor(tokenValue, {});
#if 0
            qInfo() << "line:" << cacheLine;
            qInfo() << "charStart:" << val.start.character;
            qInfo() << "charLength:" << val.length;
            qInfo() << "text:" << sourceText;
            qInfo() << "tokenType:" << val.tokenType;
            qInfo() << "tokenModifiers:" << val.tokenModifiers;
            qInfo() << "tokenValue:" << tokenValue;
#endif
            docTokenList.append({ startPos, sourceText, color });
        }
    }

    Q_EMIT highlightToken(docTokenList);
}
