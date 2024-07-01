// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codecompletionmodel.h"
#include "gui/texteditor.h"
#include "lsp/languageclienthandler.h"

CompletionSortFilterProxyModel::CompletionSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool CompletionSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
    if (!index.isValid())
        return false;

    const QRegExp regexp = filterRegExp();
    if (regexp.pattern().isEmpty() || sourceModel()->rowCount(index) > 0)
        return true;

    const QString filterText = index.data(CodeCompletionModel::FilterTextRole).toString();
    return filterText.contains(regexp);
}

bool CompletionSortFilterProxyModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    auto leftStr = sourceLeft.data(CodeCompletionModel::SortTextRole).toString();
    auto rightStr = sourceRight.data(CodeCompletionModel::SortTextRole).toString();
    bool ret = leftStr < rightStr;

    const QRegExp regexp = filterRegExp();
    if (regexp.pattern().isEmpty())
        return ret;

    auto leftTextStr = sourceLeft.data(CodeCompletionModel::FilterTextRole).toString();
    auto rightTextStr = sourceRight.data(CodeCompletionModel::FilterTextRole).toString();
    bool leftStartsWith = leftTextStr.startsWith(regexp.pattern(), Qt::CaseInsensitive);
    bool rightStartsWith = rightTextStr.startsWith(regexp.pattern(), Qt::CaseInsensitive);
    if (leftStartsWith && !rightStartsWith)
        return true;
    else if (!leftStartsWith && rightStartsWith)
        return false;

    return ret;
}

class CodeCompletionModelPrivate
{
public:
    explicit CodeCompletionModelPrivate() {}

    QIcon iconForKind(lsp::CompletionItem::Kind k);

    QList<lsp::CompletionItem> completionDatas;
    bool hasGroups = false;
};

QIcon CodeCompletionModelPrivate::iconForKind(lsp::CompletionItem::Kind k)
{
    switch (k) {
    case lsp::CompletionItem::Method:
    case lsp::CompletionItem::Function:
    case lsp::CompletionItem::Constructor:
        return QIcon::fromTheme("func");
    case lsp::CompletionItem::Class:
    case lsp::CompletionItem::Interface:
    case lsp::CompletionItem::Struct:
        return QIcon::fromTheme("class");
    case lsp::CompletionItem::Module:
        return QIcon::fromTheme("namespace");
    case lsp::CompletionItem::Field:
    case lsp::CompletionItem::Property:
    case lsp::CompletionItem::Variable:
        return QIcon::fromTheme("var");
    case lsp::CompletionItem::Enum:
    case lsp::CompletionItem::EnumMember:
        return QIcon::fromTheme("enum");
    case lsp::CompletionItem::Keyword:
        return QIcon::fromTheme("keyword");
    case lsp::CompletionItem::Snippet:
        return QIcon::fromTheme("snippet");
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
    connect(editor->languageClient(), &LanguageClientHandler::completeFinished, this, &CodeCompletionModel::onCompleteFinished, Qt::UniqueConnection);

    beginResetModel();
    d->completionDatas.clear();

    int line = 0, col = 0;
    editor->lineIndexFromPosition(position, &line, &col);
    editor->languageClient()->requestCompletion(line, col);

    endResetModel();
}

lsp::Range CodeCompletionModel::range() const
{
    if (d->completionDatas.isEmpty())
        return {};

    return d->completionDatas.first().textEdit.range;
}

lsp::CompletionItem *CodeCompletionModel::item(const QModelIndex &index) const
{
    return static_cast<lsp::CompletionItem *>(index.internalPointer());
}

int CodeCompletionModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 2;
}

QModelIndex CodeCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= rowCount(parent) || column < 0 || column >= 2 || parent.isValid())
        return QModelIndex();

    return createIndex(row, column, &d->completionDatas[row]);
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
    case NameRole:
        return item.label;
    case IconRole:
        return d->iconForKind(item.kind);
    case InsertTextRole:
        return item.insertText;
    case KindRole:
        return item.kind;
    case SortTextRole:
        return item.sortText;
    case FilterTextRole:
        return item.filterText;
    default:
        break;
    }

    return QVariant();
}

void CodeCompletionModel::onCompleteFinished(const lsp::CompletionProvider &provider)
{
    beginResetModel();
    d->completionDatas = provider.items;
    endResetModel();
}
