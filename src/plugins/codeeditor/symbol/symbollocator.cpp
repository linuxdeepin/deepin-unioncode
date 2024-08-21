// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbollocator.h"
#include "symbolmanager.h"
#include "gui/workspacewidget.h"
#include "gui/tabwidget.h"
#include "gui/texteditor.h"

#include <QUuid>
#include <QRegularExpression>

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
    void createSymbolItem(const newlsp::DocumentSymbol &symbol);
    void createSymbolItem(const newlsp::SymbolInformation &info);

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
    
    return tabWidget->currentEditor();
}

void SymbolLocatorPrivate::createSymbolItem(const newlsp::DocumentSymbol &symbol)
{
    SymbolLocatorItem item(q);
    item.id = QUuid::createUuid().toString();
    item.name = symbol.name;
    item.displayName = SymbolManager::instance()->displayNameFromDocumentSymbol(static_cast<SymbolManager::SymbolKind>(symbol.kind),
                                                                                symbol.name,
                                                                                symbol.detail.value_or(QString()));
    item.tooltip = item.displayName;
    item.line = symbol.range.start.line;
    item.column = symbol.range.start.character;
    item.icon = SymbolManager::instance()->iconFromKind(static_cast<SymbolManager::SymbolKind>(symbol.kind));
    itemList.append(item);

    auto children = symbol.children.value_or(QList<newlsp::DocumentSymbol>());
    for (const auto &child : children) {
        createSymbolItem(child);
    }
}

void SymbolLocatorPrivate::createSymbolItem(const newlsp::SymbolInformation &info)
{
    SymbolLocatorItem item(q);
    item.id = QUuid::createUuid().toString();
    item.name = item.displayName = item.tooltip = info.name;
    item.line = info.location.range.start.line;
    item.column = info.location.range.start.character;
    item.icon = SymbolManager::instance()->iconFromKind(static_cast<SymbolManager::SymbolKind>(info.kind));

    itemList.append(item);
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

    const auto &symbolList = SymbolManager::instance()->documentSymbols(editor->getFile());
    if (!symbolList.isEmpty()) {
        for (const auto &symbol : symbolList) {
            d->createSymbolItem(symbol);
        }
    } else {
        const auto &infoList = SymbolManager::instance()->symbolInformations(editor->getFile());
        for (const auto &info : infoList) {
            d->createSymbolItem(info);
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
