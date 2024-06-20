// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codelenstree.h"
#include "framework.h"
#include "codelensdelegate.h"
#include <QStandardItem>
#include <QFile>

QByteArray readLine(const QString &filePath, int line)
{
    QByteArray array{};
    QFile file(filePath);
    if(!file.open(QFile::ReadOnly)) {
        return array;
    }

    for(int i = 0; i <= line; i++) {
        array = file.readLine();
        if (i == line)
            return array;
    }
    file.close();
    return array;
}

CodeLensTree::CodeLensTree(QWidget *parent)
    : QTreeView (parent)
{
    setModel(new QStandardItemModel);
    setEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setItemDelegate(new CodeLensDelegate);
    setHeaderHidden(true);
    setLineWidth(0);

    QObject::connect(this, &QTreeView::doubleClicked, [=](const QModelIndex &index){
        if (!index.parent().isValid()) { //root return
            return;
        }
        QVariant rangeVar = index.data(CodeLensItemRole::Range);
        lsp::Range range;
        if (rangeVar.canConvert<lsp::Range>()){
            range = rangeVar.value<lsp::Range>();
        }
        QModelIndex parentIndex = index;
        while (parentIndex.parent().isValid()) {
            parentIndex = index.parent();
        }
        QString filePath = parentIndex.data(Qt::DisplayRole).toString();
        emit CodeLensTree::doubleClicked(filePath, range);
    });
}

QString codeDataFormat(int line, const QString &codeText)
{
    return QString::number(line) + " " + codeText;
}

void CodeLensTree::setData(const lsp::References &refs)
{
    auto model = qobject_cast<QStandardItemModel*>(CodeLensTree::model());
    model->clear();
    QHash<QString, QStandardItem*> cache{};
    for(auto ref : refs) {
        QString file = ref.fileUrl.toLocalFile();
        lsp::Range range = ref.range;
        if (range.start.line == range.end.line) {
            QString filePath = ref.fileUrl.toLocalFile();
            QStandardItem *fileItem = nullptr;
            if (cache[filePath]) {
                fileItem = cache[filePath];
            } else {
                fileItem = new QStandardItem(filePath);
                cache[filePath] = fileItem;
                model->appendRow(fileItem);
            }
            QString codeText = readLine(file, range.start.line);
            QString displayText = codeDataFormat(range.start.line, codeText);
            QColor hColor(Qt::yellow);
            QStandardItem *codeChild = new QStandardItem(displayText);
            codeChild->setData(QVariant::fromValue<lsp::Range>(range), CodeLensItemRole::Range);
            codeChild->setData(QVariant::fromValue<QString>(codeText), CodeLensItemRole::CodeText);
            codeChild->setData(QVariant::fromValue<QColor>(hColor), CodeLensItemRole::HeightColor);
            codeChild->setTextAlignment(Qt::AlignVCenter);
            fileItem->appendRow(codeChild);
        }
    }
}
