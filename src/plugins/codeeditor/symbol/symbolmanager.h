// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLMANAGER_H
#define SYMBOLMANAGER_H

#include <QIcon>

#include "common/lsp/protocol/new/languagefeatures.h"

class SymbolManagerPrivate;
class SymbolManager : public QObject
{
    Q_OBJECT
public:
    enum SymbolKind {
        File = 1,
        Module = 2,
        Namespace = 3,
        Package = 4,
        Class = 5,
        Method = 6,
        Property = 7,
        Field = 8,
        Constructor = 9,
        Enum = 10,
        Interface = 11,
        Function = 12,
        Variable = 13,
        Constant = 14,
        String = 15,
        Number = 16,
        Boolean = 17,
        Array = 18,
        Object = 19,
        Key = 20,
        Null = 21,
        EnumMember = 22,
        Struct = 23,
        Event = 24,
        Operator = 25,
        TypeParameter = 26
    };

    struct SymbolInfo
    {
        QString symbolName;
        SymbolKind kind;
        newlsp::Range range;
    };

    static SymbolManager *instance();

    void setDocumentSymbols(const QString &file, const QList<newlsp::DocumentSymbol> &docSymbols);
    QList<newlsp::DocumentSymbol> documentSymbols(const QString &file) const;
    void setSymbolInformations(const QString &file, const QList<newlsp::SymbolInformation> &symbolInfos);
    QList<newlsp::SymbolInformation> symbolInformations(const QString &file) const;
    SymbolInfo findSymbol(const QString &file, int line, int column);

    QIcon iconFromKind(SymbolKind kind);
    QString displayNameFromDocumentSymbol(SymbolKind kind, const QString &name, const QString &detail);

Q_SIGNALS:
    void symbolChanged(const QString &file);

private:
    SymbolManager(QObject *parent = nullptr);
    ~SymbolManager();

    SymbolManagerPrivate *const d;
    friend class SymbolManagerPrivate;
};

#endif   // SYMBOLMANAGER_H
