// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODECOMPLETIONMODEL_H
#define CODECOMPLETIONMODEL_H

#include "common/common.h"

#include <QAbstractItemModel>
#include <QKeySequence>

class TextEditor;
class CodeCompletionModelPrivate;
class CodeCompletionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CodeCompletionModel(QObject *parent = nullptr);
    ~CodeCompletionModel() override;

    enum Columns {
        Icon,
        Name
    };
    static const int ColumnCount = Name + 1;

    void clear();
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
