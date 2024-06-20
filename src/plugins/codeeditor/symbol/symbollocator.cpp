// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbollocator.h"
#include "gui/workspacewidget.h"
#include "gui/tabwidget.h"
#include "gui/texteditor.h"

#include <QUuid>
#include <QRegularExpression>

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

struct SymbolLocatorItem : public baseLocatorItem
{
    explicit SymbolLocatorItem(abstractLocator *parentLocator)
        : baseLocatorItem(parentLocator) {}

    int line = -1;
    int column = -1;
    QString name;
};

class SymbolLocatorPrivate
{
public:
    explicit SymbolLocatorPrivate(SymbolLocator *qq);

    TextEditor *currentEditor();
    QIcon fromKind(SymbolKind kind);
    SymbolLocatorItem createSymbolItem(const newlsp::DocumentSymbol &symbol);
    SymbolLocatorItem createSymbolItem(const newlsp::SymbolInformation &info);
    QString displayNameFromDocumentSymbol(SymbolKind kind, const QString &name, const QString &detail);

public:
    SymbolLocator *q;
    WorkspaceWidget *workspace { nullptr };
    QList<SymbolLocatorItem> itemList;
};

SymbolLocatorPrivate::SymbolLocatorPrivate(SymbolLocator *qq)
    : q(qq)
{
}

TextEditor *SymbolLocatorPrivate::currentEditor()
{
    auto tabWidget = workspace->currentTabWidget();
    if (!tabWidget)
        return nullptr;

    return qobject_cast<TextEditor *>(tabWidget->currentWidget());
}

QIcon SymbolLocatorPrivate::fromKind(SymbolKind kind)
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

SymbolLocatorItem SymbolLocatorPrivate::createSymbolItem(const newlsp::DocumentSymbol &symbol)
{
    SymbolLocatorItem item(q);
    item.id = QUuid::createUuid().toString();
    item.name = symbol.name;
    item.displayName = displayNameFromDocumentSymbol(static_cast<SymbolKind>(symbol.kind),
                                                     symbol.name,
                                                     symbol.detail.value_or(QString()));
    item.tooltip = item.displayName;
    item.line = symbol.range.start.line;
    item.column = symbol.range.start.character;
    item.icon = fromKind(static_cast<SymbolKind>(symbol.kind));

    return item;
}

SymbolLocatorItem SymbolLocatorPrivate::createSymbolItem(const newlsp::SymbolInformation &info)
{
    SymbolLocatorItem item(q);
    item.id = QUuid::createUuid().toString();
    item.name = item.displayName = item.tooltip = info.name;
    item.line = info.location.range.start.line;
    item.column = info.location.range.start.character;
    item.icon = fromKind(static_cast<SymbolKind>(info.kind));

    return item;
}

QString SymbolLocatorPrivate::displayNameFromDocumentSymbol(SymbolKind kind, const QString &name, const QString &detail)
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

SymbolLocator::SymbolLocator(QObject *parent)
    : abstractLocator(parent),
      d(new SymbolLocatorPrivate(this))
{
    setDisplayName("@");
    setDescription(tr("Symbols in Current Document"));
    setIncludedDefault(false);
}

SymbolLocator::~SymbolLocator()
{
    delete d;
}

void SymbolLocator::setWorkspaceWidget(WorkspaceWidget *workspace)
{
    d->workspace = workspace;
}

void SymbolLocator::prepareSearch(const QString &searchText)
{
    Q_UNUSED(searchText)

    d->itemList.clear();
    auto editor = d->currentEditor();
    if (!editor)
        return;

    const auto &symbolList = editor->documentSymbolList();
    if (!symbolList.isEmpty()) {
        for (const auto &symbol : symbolList) {
            auto item = d->createSymbolItem(symbol);
            d->itemList.append(item);

            auto children = symbol.children.value_or(QList<newlsp::DocumentSymbol>());
            for (const auto &child : children) {
                auto item = d->createSymbolItem(child);
                d->itemList.append(item);
            }
        }
    } else {
        const auto &infoList = editor->symbolInfoList();
        for (const auto &info : infoList) {
            auto item = d->createSymbolItem(info);
            d->itemList.append(item);
        }
    }
}

QList<baseLocatorItem> SymbolLocator::matchesFor(const QString &inputText)
{
    QList<baseLocatorItem> matchedResults;
    auto regexp = createRegExp(inputText);

    for (auto item : d->itemList) {
        auto match = regexp.match(item.name);
        if (match.hasMatch())
            matchedResults.append(item);
    }

    return matchedResults;
}

void SymbolLocator::accept(baseLocatorItem item)
{
    auto iter = std::find_if(d->itemList.begin(), d->itemList.end(),
                             [&](const SymbolLocatorItem &symbItem) {
                                 return symbItem.id == item.id;
                             });
    if (iter == d->itemList.end())
        return;

    if (auto editor = d->currentEditor()) {
        int pos = editor->positionFromLineIndex(iter->line, iter->column);
        editor->gotoPosition(pos);
    }
}
