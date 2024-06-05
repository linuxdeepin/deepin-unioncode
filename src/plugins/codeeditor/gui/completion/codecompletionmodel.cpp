// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codecompletionmodel.h"
#include "gui/texteditor.h"
#include "lsp/lspstyle.h"

#include <DIconTheme>

class CodeCompletionModelPrivate
{
public:
    explicit CodeCompletionModelPrivate() {}

    bool isFunctionKind(lsp::CompletionItem::Kind k);
    QIcon iconForKind(lsp::CompletionItem::Kind k);

    QList<lsp::CompletionItem> completionDatas;
    bool hasGroups = false;
};

bool CodeCompletionModelPrivate::isFunctionKind(lsp::CompletionItem::Kind k)
{
    return k == lsp::CompletionItem::Function || k == lsp::CompletionItem::Method;
}

QIcon CodeCompletionModelPrivate::iconForKind(lsp::CompletionItem::Kind k)
{
    switch (k) {
    case lsp::CompletionItem::Method:
    case lsp::CompletionItem::Function:
    case lsp::CompletionItem::Constructor:
        return DIconTheme::findQIcon("func");
    case lsp::CompletionItem::Class:
    case lsp::CompletionItem::Interface:
    case lsp::CompletionItem::Struct:
        return DIconTheme::findQIcon("class");
    case lsp::CompletionItem::Module:
        return DIconTheme::findQIcon("namespace");
    case lsp::CompletionItem::Field:
    case lsp::CompletionItem::Property:
    case lsp::CompletionItem::Variable:
        return DIconTheme::findQIcon("var");
    case lsp::CompletionItem::Enum:
    case lsp::CompletionItem::EnumMember:
        return DIconTheme::findQIcon("enum");
    case lsp::CompletionItem::Keyword:
        return DIconTheme::findQIcon("keyword");
    case lsp::CompletionItem::Snippet:
        return DIconTheme::findQIcon("snippet");
    default:
        break;
    }

    return QIcon();
}

CodeCompletionModel::CodeCompletionModel(QObject *parent)
    : QAbstractItemModel(parent),
      d(new CodeCompletionModelPrivate)
{
}

CodeCompletionModel::~CodeCompletionModel()
{
    delete d;
}

void CodeCompletionModel::clear()
{
    beginResetModel();
    d->completionDatas.clear();
    endResetModel();
}

void CodeCompletionModel::completionInvoked(TextEditor *editor, int position)
{
    connect(editor->lspStyle(), &LSPStyle::completeFinished, this, &CodeCompletionModel::onCompleteFinished, Qt::UniqueConnection);

    beginResetModel();
    d->completionDatas.clear();

    int line = 0, col = 0;
    editor->lineIndexFromPosition(position, &line, &col);
    editor->lspStyle()->requestCompletion(line, col);

    endResetModel();
}

void CodeCompletionModel::executeCompletionItem(TextEditor *editor, int start, int end, const QModelIndex &index)
{
    if (index.row() >= d->completionDatas.size())
        return;

    int line = 0, col = 0;
    editor->lineIndexFromPosition(end, &line, &col);
    int lineEndPos = editor->SendScintilla(TextEditor::SCI_GETLINEENDPOSITION, line);

    QString next;
    if (end >= lineEndPos)
        next = editor->text(lineEndPos - 1, lineEndPos);
    else
        next = editor->text(end, end + 1);

    QString matching = d->completionDatas.at(index.row()).insertText;
    if ((next == QLatin1Char('"') && matching.endsWith(QLatin1Char('"')))
        || (next == QLatin1Char('>') && matching.endsWith(QLatin1Char('>'))))
        matching.chop(1);

    auto kind = d->completionDatas.at(index.row()).kind;
    bool addParens = next != QLatin1Char('(') && d->isFunctionKind(kind);
    if (addParens)
        matching += QStringLiteral("()");

    editor->replaceRange(start, end, matching);
    if (addParens) {
        int curLine = 0, curIndex = 0;
        editor->lineIndexFromPosition(editor->cursorPosition(), &curLine, &curIndex);
        editor->setCursorPosition(curLine, curIndex - 1);
    }
}

int CodeCompletionModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return ColumnCount;
}

QModelIndex CodeCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= rowCount(parent) || column < 0 || column >= ColumnCount || parent.isValid())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex CodeCompletionModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

int CodeCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return d->completionDatas.size();
}

QVariant CodeCompletionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->completionDatas.size())
        return QVariant();

    const auto &item = d->completionDatas.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == Name)
            return item.label;
        break;
    case Qt::DecorationRole:
        return d->iconForKind(item.kind);
    default:
        break;
    }

    return QVariant();
}

void CodeCompletionModel::onCompleteFinished(const lsp::CompletionProvider &provider)
{
    beginResetModel();

    d->completionDatas = provider.items;

    qSort(d->completionDatas.begin(), d->completionDatas.end(),
          [](const lsp::CompletionItem &item1, const lsp::CompletionItem &item2) {
              return item1.sortText < item2.sortText;
          });

    endResetModel();
}
