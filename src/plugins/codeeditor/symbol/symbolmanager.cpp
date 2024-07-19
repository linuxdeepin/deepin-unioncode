// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolmanager.h"

#include <QHash>

class SymbolManagerPrivate
{
public:
    QHash<QString, QList<newlsp::DocumentSymbol>> docSymbolHash;
    QHash<QString, QList<newlsp::SymbolInformation>> symbolInfoHash;
};

SymbolManager::SymbolManager()
    : d(new SymbolManagerPrivate)
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
}

QList<newlsp::DocumentSymbol> SymbolManager::documentSymbols(const QString &file) const
{
    return d->docSymbolHash.value(file, {});
}

void SymbolManager::setSymbolInformations(const QString &file, const QList<newlsp::SymbolInformation> &symbolInfos)
{
    d->symbolInfoHash.insert(file, symbolInfos);
}

QList<newlsp::SymbolInformation> SymbolManager::symbolInformations(const QString &file) const
{
    return d->symbolInfoHash.value(file, {});
}

QString SymbolManager::symbolName(const QString &file, int line, int column)
{
    if (d->docSymbolHash.contains(file)) {
        const auto &symbolList = d->docSymbolHash[file];
        for (const auto &symbol : symbolList) {
            if (!symbol.range.contains({ line, column }))
                continue;

            auto children = symbol.children.value_or(QList<newlsp::DocumentSymbol>());
            for (const auto &child : children) {
                if (child.range.contains({ line, column }))
                    return displayNameFromDocumentSymbol(static_cast<SymbolManager::SymbolKind>(child.kind),
                                                         child.name,
                                                         child.detail.value_or(QString()));
            }
            return displayNameFromDocumentSymbol(static_cast<SymbolManager::SymbolKind>(symbol.kind),
                                                 symbol.name,
                                                 symbol.detail.value_or(QString()));

        }
    } else if (d->symbolInfoHash.contains(file)) {
        const auto &infoList = SymbolManager::instance()->symbolInformations(file);
        for (const auto &info : infoList) {
            if (info.location.range.contains({ line, column }))
                return info.name;
        }
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
    switch (kind) {
    case SymbolKind::Constructor:
        return name + detail;
    case SymbolKind::Method:
    case SymbolKind::Function: {
        const int lastParenIndex = detail.lastIndexOf(')');
        if (lastParenIndex == -1)
            return name;
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
            return name;
        return name + detail.mid(i) + " -> " + detail.left(i);
    }
    case SymbolKind::Variable:
    case SymbolKind::Field:
    case SymbolKind::Constant:
        if (detail.isEmpty())
            return name;
        return name + " -> " + detail;
    default:
        return name;
    }
}
