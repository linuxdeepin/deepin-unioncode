// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolmanager.h"

#include <QHash>

class SymbolManagerPrivate
{
public:
    explicit SymbolManagerPrivate(SymbolManager *qq);

    QPair<QString, int> findSymbol(const newlsp::DocumentSymbol &symbol, int line, int column);

public:
    SymbolManager *q;

    QHash<QString, QList<newlsp::DocumentSymbol>> docSymbolHash;
    QHash<QString, QList<newlsp::SymbolInformation>> symbolInfoHash;
};

SymbolManagerPrivate::SymbolManagerPrivate(SymbolManager *qq)
    : q(qq)
{
}

QPair<QString, int> SymbolManagerPrivate::findSymbol(const newlsp::DocumentSymbol &symbol, int line, int column)
{
    auto children = symbol.children.value_or(QList<newlsp::DocumentSymbol>());
    auto iter = std::find_if(children.cbegin(), children.cend(),
                             [&](const newlsp::DocumentSymbol &child) {
                                 return child.range.contains({ line, column });
                             });
    if (iter != children.cend())
        return findSymbol(*iter, line, column);

    QString name = q->displayNameFromDocumentSymbol(static_cast<SymbolManager::SymbolKind>(symbol.kind),
                                                    symbol.name,
                                                    symbol.detail.value_or(QString()));
    return qMakePair(name, symbol.kind);
}

SymbolManager::SymbolManager(QObject *parent)
    : QObject(parent),
      d(new SymbolManagerPrivate(this))
{
}

SymbolManager::~SymbolManager()
{
    delete d;
}

SymbolManager *SymbolManager::instance()
{
    static SymbolManager ins;
    return &ins;
}

void SymbolManager::setDocumentSymbols(const QString &file, const QList<newlsp::DocumentSymbol> &docSymbols)
{
    d->docSymbolHash.insert(file, docSymbols);
    Q_EMIT symbolChanged(file);
}

QList<newlsp::DocumentSymbol> SymbolManager::documentSymbols(const QString &file) const
{
    return d->docSymbolHash.value(file, {});
}

void SymbolManager::setSymbolInformations(const QString &file, const QList<newlsp::SymbolInformation> &symbolInfos)
{
    d->symbolInfoHash.insert(file, symbolInfos);
    Q_EMIT symbolChanged(file);
}

QList<newlsp::SymbolInformation> SymbolManager::symbolInformations(const QString &file) const
{
    return d->symbolInfoHash.value(file, {});
}

QPair<QString, int> SymbolManager::findSymbol(const QString &file, int line, int column)
{
    if (d->docSymbolHash.contains(file)) {
        const auto &symbolList = d->docSymbolHash[file];
        auto iter = std::find_if(symbolList.cbegin(), symbolList.cend(),
                                 [&](const newlsp::DocumentSymbol &symbol) {
                                     return symbol.range.contains({ line, column });
                                 });
        if (iter != symbolList.cend())
            return d->findSymbol(*iter, line, column);
    } else if (d->symbolInfoHash.contains(file)) {
        const auto &infoList = SymbolManager::instance()->symbolInformations(file);
        auto iter = std::find_if(infoList.cbegin(), infoList.cend(),
                                 [&](const newlsp::SymbolInformation &info) {
                                     return info.location.range.contains({ line, column });
                                 });
        if (iter != infoList.cend())
            return qMakePair(iter->name, iter->kind);
    }

    return {};
}

QIcon SymbolManager::iconFromKind(SymbolKind kind)
{
    switch (kind) {
    case SymbolKind::File:
        return QIcon::fromTheme("text-plain");
    case SymbolKind::Module:
    case SymbolKind::Namespace:
    case SymbolKind::Package:
        return QIcon::fromTheme("namespace");
    case SymbolKind::Class:
    case SymbolKind::Constructor:
    case SymbolKind::Interface:
    case SymbolKind::Object:
    case SymbolKind::Struct:
        return QIcon::fromTheme("class");
    case SymbolKind::Method:
    case SymbolKind::Function:
    case SymbolKind::Event:
    case SymbolKind::Operator:
        return QIcon::fromTheme("func");
        break;
    case SymbolKind::Property:
        return QIcon::fromTheme("snippet");
    case SymbolKind::Enum:
        return QIcon::fromTheme("enum");
        break;
    case SymbolKind::Field:
    case SymbolKind::Variable:
    case SymbolKind::Constant:
    case SymbolKind::String:
    case SymbolKind::Number:
    case SymbolKind::Boolean:
    case SymbolKind::Array:
    case SymbolKind::TypeParameter:
        return QIcon::fromTheme("var");
    case SymbolKind::Key:
    case SymbolKind::Null:
        return QIcon::fromTheme("keyword");
    case SymbolKind::EnumMember:
        return QIcon::fromTheme("enumerator");
    default:
        break;
    }

    return QIcon();
}

QString SymbolManager::displayNameFromDocumentSymbol(SymbolKind kind, const QString &name, const QString &detail)
{
    QString tmpName = name;
    int index = tmpName.indexOf("::");
    if (index != -1)
        tmpName = tmpName.mid(index + 2);

    switch (kind) {
    case SymbolKind::Constructor:
        return tmpName + detail;
    case SymbolKind::Method:
    case SymbolKind::Function: {
        const int lastParenIndex = detail.lastIndexOf(')');
        if (lastParenIndex == -1)
            return tmpName;
        int leftParensNeeded = 1;
        int i = -1;
        for (i = lastParenIndex - 1; i >= 0; --i) {
            switch (detail.at(i).toLatin1()) {
            case ')':
                ++leftParensNeeded;
                break;
            case '(':
                --leftParensNeeded;
                break;
            default:
                break;
            }
            if (leftParensNeeded == 0)
                break;
        }
        if (leftParensNeeded > 0)
            return tmpName;
        return tmpName + detail.mid(i) + " -> " + detail.left(i);
    }
    case SymbolKind::Variable:
    case SymbolKind::Field:
    case SymbolKind::Constant:
        if (detail.isEmpty())
            return tmpName;
        return tmpName + " -> " + detail;
    default:
        return tmpName;
    }
}
