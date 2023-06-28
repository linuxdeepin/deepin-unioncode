// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolkeeper.h"

#include "common/common.h"
#include "services/builder/builderservice.h"

#include <QStandardItem>

namespace {
static SymbolTreeView *tree{nullptr};
static SymbolParser *parser{nullptr};
static SymbolParseArgs parserArgs;
}

SymbolKeeper::SymbolKeeper()
{

}

SymbolKeeper *SymbolKeeper::instance()
{
    static SymbolKeeper ins;
    return &ins;
}

SymbolTreeView *SymbolKeeper::treeView()
{
    if (!tree) {
        tree = new SymbolTreeView();
        QObject::connect(tree, &SymbolTreeView::jumpToLine,
                         this, &SymbolKeeper::jumpToLine);
    }
    return tree;
}

void SymbolKeeper::doParse(const SymbolParseArgs &args)
{
    parserArgs = args;
    if (parser) {
        QObject::disconnect(parser, &SymbolParser::parseDone,
                            this, &SymbolKeeper::doParseDone);
        parser->kill();
        parser->waitForFinished();
        delete parser;
        parser = nullptr;
    }
    parser = new SymbolParser();
    parser->setArgs(args);
    parser->SymbolParser::start();
    QObject::connect(parser, &SymbolParser::parseDone,
                     this, &SymbolKeeper::doParseDone);
}

void SymbolKeeper::jumpToLine(const QString &filePath, const QString &fileLine)
{
    editor.jumpToLine(filePath, fileLine.toInt());
}

void SymbolKeeper::doParseDone(bool result)
{
    SymbolParseArgs args = parser->args();
    symbol.parseDone(args.workspace, args.language, args.storage, result);
    this->treeView()->expandAll();
    parser->kill();
    delete parser;
    parser = nullptr;
}
