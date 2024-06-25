// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "popupwidget.h"
#include "base/baseitemdelegate.h"

#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QHeaderView>
#include <QToolTip>

void locatorModel::clear()
{
    beginResetModel();
    items.clear();
    endResetModel();
}

QVariant locatorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= items.size())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == DisplayNameColumn)
            return items.at(index.row()).displayName;
        else if (index.column() == ExtraInfoColumn)
            return items.at(index.row()).extraInfo;
        break;
    case Qt::ToolTipRole:
        QToolTip::showText(QCursor::pos(), items.at(index.row()).tooltip);
        break;
    case Qt::DecorationRole:
        if (index.column() == DisplayNameColumn)
            return items[index.row()].icon;
        break;
    };
    return QVariant();
}

void locatorModel::addItems(const QList<baseLocatorItem> &items)
{
    beginInsertRows(QModelIndex(), this->items.size(), this->items.size() + items.size() - 1);
    this->items.append(items);
    endInsertRows();
}

QList<baseLocatorItem> locatorModel::getItems()
{
    return items;
}

int locatorModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return items.size();
}

int locatorModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

PopupWidget::PopupWidget(QWidget *parent)
    : DFrame(parent), tree(new DTreeView(this))
{
    tree->setFrameShape(QFrame::NoFrame);
    tree->setItemDelegate(new BaseItemDelegate(this));
    auto layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(tree);

    tree->setHeaderHidden(true);
    tree->setRootIsDecorated(false);
    tree->setUniformRowHeights(true);
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tree->header()->setStretchLastSection(true);

    connect(tree, &QTreeView::activated, this, [this](const QModelIndex &index) {
        emit this->selectIndex(index);
    });

    updateGeometry();
}

void PopupWidget::updateGeometry()
{
    Q_ASSERT(parentWidget());

    //resize header
    tree->header()->resizeSection(0, tree->width() / 3);
    tree->header()->resizeSection(1, 0);

    const int border = tree->frameWidth();
    QSize size = parentWidget()->size();

    const QRect rect(parentWidget()->mapToGlobal(QPoint(-border, parentWidget()->height() + 5)), QSize(size.width(), 300));

    setGeometry(rect);
}

void PopupWidget::selectFirstRow()
{
    if (model && (model->rowCount(QModelIndex()) > 0))
        tree->setCurrentIndex(model->index(0, 0));
}

void PopupWidget::previous()
{
    int index = tree->currentIndex().row();
    --index;
    if (index < 0) {
        // wrap
        index = model->rowCount(QModelIndex()) - 1;
    }
    tree->setCurrentIndex(model->index(index, 0));
}

void PopupWidget::next()
{
    int index = tree->currentIndex().row();
    ++index;
    if (index >= model->rowCount(QModelIndex())) {
        // wrap
        index = 0;
    }
    tree->setCurrentIndex(model->index(index, 0));
}

bool PopupWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Show)
        // make sure the popup has correct position before it becomes visible
        updateGeometry();
    return QWidget::event(event);
}

bool PopupWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize)
        updateGeometry();
    return QWidget::eventFilter(watched, event);
}

void PopupWidget::setmodel(locatorModel *model)
{
    tree->setModel(model);
    this->model = model;
}

void PopupWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Tab:
    case Qt::Key_Down:
        next();
        return;
    case Qt::Key_Backtab:
    case Qt::Key_Up:
        previous();
        return;
    case Qt::Key_P:
    case Qt::Key_N:
        if (event->modifiers() == Qt::Modifier::CTRL) {
            if (event->key() == Qt::Key_P)
                previous();
            else
                next();
            return;
        }
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() == 0) {
            emit selectIndex(tree->currentIndex());
            return;
        }
        break;
    }
    DFrame::keyPressEvent(event);
}
