// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODECOMPLETIONMODEL_H
#define CODECOMPLETIONMODEL_H

#include "common/common.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

class CompletionSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit CompletionSortFilterProxyModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
};

class TextEditor;
class CodeCompletionModelPrivate;
class CodeCompletionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CodeCompletionModel(QObject *parent = nullptr);
    ~CodeCompletionModel() override;

    enum ItemRole {
        IconRole = Qt::UserRole + 1,
        NameRole,
        InsertTextRole,
        KindRole,
        SortTextRole,
        FilterTextRole
    };

    void clear();
    void completionInvoked(TextEditor *editor, int position);
    lsp::Range range() const;
    lsp::CompletionItem *item(const QModelIndex &index) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private slots:
    void onCompleteFinished(const lsp::CompletionProvider &provider);

private:
    class CodeCompletionModelPrivate *const d;
};

#endif   // CODECOMPLETIONMODEL_H
