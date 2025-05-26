// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
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
        int startPos = textEditor->positionFromLineIndex(cacheLine, cacheColumn);
        int endPos = startPos + val.length;
        int lineEndPos = textEditor->SendScintilla(TextEditor::SCI_GETLINEENDPOSITION, cacheLine);
        if (endPos > lineEndPos)
            break;

        QString sourceText = textEditor->text(startPos, endPos);
        if (!sourceText.isEmpty()) {
            QString tokenValue = clientHandler->tokenToDefine(val.tokenType);
            if (tokenValue.startsWith("operator")) {
                QRegularExpression re("^[A-Za-z]+$");
                if (re.match(sourceText).hasMatch())
                    continue;
            }
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
