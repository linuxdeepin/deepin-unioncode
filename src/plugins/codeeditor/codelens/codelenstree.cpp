// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codelenstree.h"
#include "framework.h"
#include "codelensdelegate.h"

#include "services/editor/editorservice.h"

#include <QStandardItem>
#include <QFile>

using namespace dpfservice;

CodeLensTree::CodeLensTree(QWidget *parent)
    : QTreeView(parent)
{
    setModel(new QStandardItemModel(this));
    setEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setItemDelegate(new CodeLensDelegate(this));
    setHeaderHidden(true);
    setLineWidth(0);

    QObject::connect(this, &QTreeView::doubleClicked, [=](const QModelIndex &index) {
        if (!index.parent().isValid())   //root return
            return;

        int line = index.data(CodeLensItemRole::LineRole).toInt();
        int column = index.data(CodeLensItemRole::TermStartRole).toInt();
        QModelIndex parentIndex = index;
        while (parentIndex.parent().isValid()) {
            parentIndex = index.parent();
        }
        QString filePath = parentIndex.data(Qt::DisplayRole).toString();
        emit CodeLensTree::doubleClicked(filePath, line, column);
    });
}

QString codeDataFormat(int line, const QString &codeText)
{
    return QString::number(line) + " " + codeText;
}

void CodeLensTree::setData(const lsp::References &refs)
{
    onceReadFlag = false;
    auto model = qobject_cast<QStandardItemModel *>(CodeLensTree::model());
    model->clear();
    QHash<QString, QStandardItem *> cache {};
    for (auto ref : refs) {
        lsp::Range range = ref.range;
        if (range.start.line != range.end.line)
            continue;

        QString filePath = ref.fileUrl.toLocalFile();
        QStandardItem *fileItem = nullptr;
        if (cache.contains(filePath)) {
            fileItem = cache[filePath];
        } else {
            fileItem = new QStandardItem(filePath);
            cache[filePath] = fileItem;
            model->appendRow(fileItem);
        }

        QString text = readLine(filePath, range.start.line);
        if (text.isEmpty())
            continue;

        QStandardItem *item = new QStandardItem(text);
        item->setData(range.start.line, CodeLensItemRole::LineRole);
        item->setData(range.start.character, CodeLensItemRole::TermStartRole);
        item->setData(range.end.character, CodeLensItemRole::TermEndRole);
        item->setTextAlignment(Qt::AlignVCenter);
        fileItem->appendRow(item);
    }
}

QString CodeLensTree::readLine(const QString &filePath, int line)
{
    if (line < 0)
        return {};

    if (!editSrv)
        editSrv = dpfGetService(EditorService);

    static QStringList openedFileList;
    if (!onceReadFlag) {
        openedFileList = editSrv->openedFiles();
        onceReadFlag = true;
    }

    if (!openedFileList.contains(filePath))
        return readFileLine(filePath, line);

    auto fileText = editSrv->fileText(filePath);
    auto textLines = fileText.split('\n');
    if (textLines.size() > line)
        return textLines.at(line);

    return {};
}

QString CodeLensTree::readFileLine(const QString &filePath, int line)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return {};

    QTextStream in(&file);
    int lineCount = 0;
    while (!in.atEnd()) {
        QString text = in.readLine();
        if (line == lineCount)
            return text;
        lineCount++;
    }

    file.close();
    return {};
}
