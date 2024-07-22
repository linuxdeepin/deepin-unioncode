// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolview.h"
#include "symbolmanager.h"

#include "common/util/eventdefinitions.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

enum SymbolItemRole {
    FilePathRole = Qt::UserRole + 10,
    LineRole,
    ColumnRole
};

class SymbolViewPrivate : public QObject
{
public:
    explicit SymbolViewPrivate(SymbolView *qq);

    void initUI();
    void initConnection();

    QStandardItem *createSymbolItem(const QString &path, const newlsp::DocumentSymbol &symbol);
    void handleItemClicked(const QModelIndex &index);

public:
    SymbolView *q;

    QTreeView *view { nullptr };
    QFileSystemModel pathModel;
    QStandardItemModel symbolModel;
};

SymbolViewPrivate::SymbolViewPrivate(SymbolView *qq)
    : q(qq)
{
}

void SymbolViewPrivate::initUI()
{
    q->setFixedWidth(400);
    q->setWindowFlags(Qt::Popup);

    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    view = new QTreeView(q);
    view->setHeaderHidden(true);
    view->setFrameShape(QFrame::NoFrame);
    view->setEditTriggers(QTreeView::NoEditTriggers);
    view->setIconSize({ 16, 16 });

    layout->addWidget(view);
}

void SymbolViewPrivate::initConnection()
{
    connect(view, &QTreeView::clicked, this, &SymbolViewPrivate::handleItemClicked);
}

QStandardItem *SymbolViewPrivate::createSymbolItem(const QString &path, const newlsp::DocumentSymbol &symbol)
{
    QStandardItem *item = new QStandardItem();
    const auto &name = SymbolManager::instance()->displayNameFromDocumentSymbol(static_cast<SymbolManager::SymbolKind>(symbol.kind),
                                                                                symbol.name,
                                                                                symbol.detail.value_or(QString()));
    item->setText(name);
    item->setToolTip(name);
    item->setIcon(SymbolManager::instance()->iconFromKind(static_cast<SymbolManager::SymbolKind>(symbol.kind)));
    item->setData(path, FilePathRole);
    item->setData(symbol.range.start.line, LineRole);
    item->setData(symbol.range.start.character, ColumnRole);

    auto children = symbol.children.value_or(QList<newlsp::DocumentSymbol>());
    for (const auto &child : children) {
        auto childItem = createSymbolItem(path, child);
        item->appendRow(childItem);
    }

    return item;
}

void SymbolViewPrivate::handleItemClicked(const QModelIndex &index)
{
    auto path = index.data(QFileSystemModel::FilePathRole).toString();
    if (path.isEmpty()) {
        path = index.data(FilePathRole).toString();
        if (path.isEmpty())
            return;

        int line = index.data(LineRole).toInt();
        int col = index.data(ColumnRole).toInt();
        editor.gotoPosition(path, line + 1, col);
    } else {
        if (QFileInfo(path).isDir()) {
            view->expand(index);
            return;
        }

        editor.openFile(QString(), path);
    }

    q->hide();
}

SymbolView::SymbolView(QWidget *parent)
    : DFrame(parent),
      d(new SymbolViewPrivate(this))
{
    d->initUI();
    d->initConnection();
}

void SymbolView::setRootPath(const QString &path)
{
    d->view->setModel(&d->pathModel);
    d->pathModel.setRootPath(path);
    auto index = d->pathModel.index(path);
    d->view->setRootIndex(index);
    d->view->sortByColumn(0, Qt::SortOrder::AscendingOrder);

    d->view->setColumnHidden(1, true);
    d->view->setColumnHidden(2, true);
    d->view->setColumnHidden(3, true);
}

bool SymbolView::setSymbolPath(const QString &path)
{
    d->view->setModel(&d->symbolModel);
    d->symbolModel.clear();
    const auto &docSymbolList = SymbolManager::instance()->documentSymbols(path);
    if (docSymbolList.isEmpty()) {
        const auto &symbolInfoList = SymbolManager::instance()->symbolInformations(path);
        if (symbolInfoList.isEmpty())
            return false;

        for (const auto &info : symbolInfoList) {
            QStandardItem *item = new QStandardItem(info.name);
            item->setToolTip(info.name);
            item->setIcon(SymbolManager::instance()->iconFromKind(static_cast<SymbolManager::SymbolKind>(info.kind)));
            item->setData(path, FilePathRole);
            item->setData(info.location.range.start.line, LineRole);
            item->setData(info.location.range.start.character, ColumnRole);
            d->symbolModel.appendRow(item);
        }
    } else {
        for (const auto &symbol : docSymbolList) {
            QStandardItem *item = d->createSymbolItem(path, symbol);
            d->symbolModel.appendRow(item);
        }
    }

    return true;
}

void SymbolView::show(const QPoint &pos)
{
    show();
    move(pos);
    d->view->setFocus();
}

void SymbolView::hideEvent(QHideEvent *event)
{
    Q_EMIT hidden();
    DFrame::hideEvent(event);
}
