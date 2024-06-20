// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODECOMPLETIONMODEL_H
#define CODECOMPLETIONMODEL_H

#include "common/common.h"

#include <QAbstractItemModel>

class TextEditor;
class CodeCompletionModelPrivate;
class CodeCompletionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CodeCompletionModel(QObject *parent = nullptr);
    ~CodeCompletionModel() override;

    enum Columns {
        Prefix = 0,
        /// Icon representing the type of completion. We have a separate icon field
        /// so that names remain aligned where only some completions have icons,
        /// and so that they can be rearranged by the user.
        Icon,
        Scope,
        Name,
        Arguments,
        Postfix
    };
    static const int ColumnCount = Postfix + 1;

    void completionInvoked(TextEditor *editor, int position);
    void executeCompletionItem(TextEditor *editor, int start, int end, const QModelIndex &index);

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
