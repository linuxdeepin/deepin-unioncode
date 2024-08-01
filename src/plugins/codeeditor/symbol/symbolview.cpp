// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolview.h"
#include "symbolmanager.h"

#include "common/util/eventdefinitions.h"
#include "base/baseitemdelegate.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

enum SymbolItemRole {
    FilePathRole = Qt::UserRole + 10,
    SymbolRangeRole
};

enum Mode {
    Symbol = 0,
    FileSystem
};

class SymbolViewPrivate : public QObject
{
public:
    explicit SymbolViewPrivate(SymbolView *qq);

    void initUI();
    void initConnection();

    QStandardItem *createSymbolItem(const QString &path, const newlsp::DocumentSymbol &symbol);
    void handleItemClicked(const QModelIndex &index);
    void select(const QModelIndex &index);

public:
    SymbolView *q;

    QTreeView *view { nullptr };
    QFileSystemModel pathModel;
    QStandardItemModel symbolModel;
    Mode currentMode { Symbol };
    SymbolView::ClickMode clickMode { SymbolView::Click };
    bool clickToHide { false };
};

SymbolViewPrivate::SymbolViewPrivate(SymbolView *qq)
    : q(qq)
{
}

void SymbolViewPrivate::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    view = new QTreeView(q);
    view->setHeaderHidden(true);
    view->setFrameShape(QFrame::NoFrame);
    view->setEditTriggers(QTreeView::NoEditTriggers);
    view->setIconSize({ 16, 16 });
    view->installEventFilter(q);
    view->viewport()->installEventFilter(q);
    view->setItemDelegate(new BaseItemDelegate(view));

    layout->addWidget(view);
}

void SymbolViewPrivate::initConnection()
{
    if (clickMode == SymbolView::Click)
        connect(view, &QTreeView::clicked, this, &SymbolViewPrivate::handleItemClicked);
    else
        connect(view, &QTreeView::doubleClicked, this, &SymbolViewPrivate::handleItemClicked);
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
    item->setData(QVariant::fromValue(symbol.range), SymbolRangeRole);

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

        const auto &range = index.data(SymbolRangeRole).value<newlsp::Range>();
        editor.gotoPosition(path, range.start.line + 1, range.start.character);
    } else {
        if (QFileInfo(path).isDir()) {
            view->setExpanded(index, !view->isExpanded(index));
            return;
        }

        editor.openFile(QString(), path);
    }

    if (clickToHide)
        q->hide();
}

void SymbolViewPrivate::select(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    view->setCurrentIndex(index);
    view->scrollTo(index, QTreeView::PositionAtCenter);
}

SymbolView::SymbolView(ClickMode mode, QWidget *parent)
    : DFrame(parent),
      d(new SymbolViewPrivate(this))
{
    d->clickMode = mode;
    d->initUI();
    d->initConnection();
}

void SymbolView::setRootPath(const QString &path)
{
    d->currentMode = FileSystem;
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
    d->currentMode = Symbol;
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
            item->setData(QVariant::fromValue(info.location.range), SymbolRangeRole);
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

void SymbolView::select(const QString &text)
{
    QModelIndex index;
    if (d->currentMode == FileSystem) {
        index = d->pathModel.index(text);
    } else {
        const auto &itemList = d->symbolModel.findItems(text, Qt::MatchExactly | Qt::MatchRecursive);
        if (itemList.isEmpty())
            return;
        index = d->symbolModel.indexFromItem(itemList.first());
    }

    d->select(index);
}

void SymbolView::selectSymbol(const QString &symbol, int line, int col)
{
    const auto &itemList = d->symbolModel.findItems(symbol, Qt::MatchExactly | Qt::MatchRecursive);
    if (itemList.isEmpty())
        return;

    auto iter = std::find_if(itemList.crbegin(), itemList.crend(),
                             [&](QStandardItem *item) {
                                 const auto &range = item->data(SymbolRangeRole).value<newlsp::Range>();
                                 return range.contains({ line, col });
                             });

    if (iter != itemList.crend()) {
        const auto &index = d->symbolModel.indexFromItem(*iter);
        d->select(index);
    }
}

void SymbolView::expandAll()
{
    d->view->expandAll();
}

void SymbolView::collapseAll()
{
    d->view->collapseAll();
}

bool SymbolView::isEmpty()
{
    if (d->currentMode == Symbol)
        return d->symbolModel.rowCount() == 0;

    return d->pathModel.rowCount() == 0;
}

void SymbolView::setClickToHide(bool enable)
{
    d->clickToHide = enable;
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

bool SymbolView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->view && event->type() == QEvent::KeyPress) {
        auto keyEvent = dynamic_cast<QKeyEvent *>(event);
        if (keyEvent && (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)) {
            d->handleItemClicked(d->view->currentIndex());
            return true;
        }
    }

    if (watched == d->view->viewport() && d->clickMode == Click && event->type() == QEvent::MouseButtonDblClick)
        return true;

    return DFrame::eventFilter(watched, event);
}
