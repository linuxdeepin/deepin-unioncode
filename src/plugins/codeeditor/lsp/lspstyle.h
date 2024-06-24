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

    void requestCompletion(int line, int column);
    void updateTokens();
    void refreshTokens();
    QList<newlsp::DocumentSymbol> documentSymbolList() const;
    QList<newlsp::SymbolInformation> symbolInformationList() const;

public slots:
    void switchHeaderSource(const QString &file);
    void followSymbolUnderCursor();
    void findUsagesActionTriggered();
    void renameActionTriggered();
    void formatSelections();

signals:
    void completeFinished(const lsp::CompletionProvider &provider);

private:
    QSharedPointer<LSPStylePrivate> d { nullptr };
};

#endif   // LSPSTYLE_H
