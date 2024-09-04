// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "referencepopup.h"
#include "base/baseitemdelegate.h"

#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QHeaderView>
#include <QToolTip>
#include <QPainter>
#include <QFileInfo>

#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif
#include <DGuiApplicationHelper>

inline constexpr int kRectRadius = { 8 };
inline constexpr int kTextLeftMargin = { 8 };

DisplayItemDelegate::DisplayItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void DisplayItemDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);

    painter->setRenderHints(painter->renderHints()
                            | QPainter::Antialiasing
                            | QPainter::TextAntialiasing
                            | QPainter::SmoothPixmapTransform);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    paintItemBackground(painter, opt, index);
    paintItemColumn(painter, opt, index);
}

QSize DisplayItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    if (index.isValid()) {
        QSize size = index.data(Qt::SizeHintRole).toSize();
        if (size.isValid()) {
            return size;
        } else {
            return { option.rect.width(), option.fontMetrics.height() * 2 + 5 };
        }
    }
    return DStyledItemDelegate::sizeHint(option, index);
}

void DisplayItemDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    painter->save();

    if (option.widget) {
#ifdef DTKWIDGET_CLASS_DPaletteHelper
        DPalette pl(DPaletteHelper::instance()->palette(option.widget));
#else
        DPalette pl = DGuiApplicationHelper::instance()->applicationPalette();
#endif
        QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
        QColor adjustColor = baseColor;

        bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
        if (isSelected) {
            adjustColor = option.palette.color(DPalette::ColorGroup::Active, QPalette::Highlight);
        } else if (option.state & QStyle::StateFlag::State_MouseOver) {
            // hover color
            adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        } else {
            painter->setOpacity(0);
        }

        // set paint path
        QPainterPath path;
        path.addRoundedRect(option.rect, kRectRadius, kRectRadius);
        painter->fillPath(path, adjustColor);
    }

    painter->restore();
}

void DisplayItemDelegate::paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    painter->save();

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected)
        painter->setPen(option.palette.color(DPalette::ColorGroup::Active, QPalette::HighlightedText));

    auto model = dynamic_cast<const ItemModel *>(index.model());
    if (!model)
        return;
    auto items = model->getItems();
    auto itemInfo = items.at(index.row());
    QString filePath = itemInfo.extraInfo;
    QString fileName = itemInfo.displayName;
    QFontMetrics fm(option.font);

    QString elidFilePath;
    elidFilePath = fm.elidedText(filePath, Qt::ElideMiddle, option.rect.width());

    painter->drawText(option.rect.adjusted(kTextLeftMargin, 5, 0, -5),
                      Qt::AlignLeft | Qt::AlignBottom, elidFilePath);

    QString elidFileName;
    elidFileName = fm.elidedText(fileName, Qt::ElideMiddle, option.rect.width());
    painter->drawText(option.rect.adjusted(kTextLeftMargin, 5, 0, -5),
                      Qt::AlignLeft | Qt::AlignTop, elidFileName);

    painter->restore();
}

void ItemModel::clear()
{
    beginResetModel();
    items.clear();
    endResetModel();
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
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
        if (!items.at(index.row()).extraInfo.isEmpty())
            return items.at(index.row()).extraInfo;
    case Qt::DecorationRole:
        if (index.column() == DisplayNameColumn)
            return items[index.row()].icon;
        break;
    };
    return QVariant();
}

void ItemModel::addItems(const QList<ItemInfo> &items)
{
    beginInsertRows(QModelIndex(), this->items.size(), this->items.size() + items.size() - 1);
    this->items.append(items);
    endInsertRows();
}

QList<ItemInfo> ItemModel::getItems() const
{
    return items;
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return items.size();
}

int ItemModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    for (auto item : items)
        if (!item.extraInfo.isEmpty())
            return ColumnCount;
    return 1;
}

PopupWidget::PopupWidget(QWidget *parent)
    : DBlurEffectWidget(parent), list(new DListView(this))
{
    list->setFrameShape(QFrame::NoFrame);
    list->setItemDelegate(new BaseItemDelegate(this));
    auto layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(list);

    list->setAutoFillBackground(false);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setEditTriggers(QListView::NoEditTriggers);
    list->setSizeAdjustPolicy(QAbstractItemView::AdjustToContents);
    list->setAlternatingRowColors(false);
    list->setSelectionRectVisible(true);
    list->setUniformItemSizes(true);
    list->setOrientation(QListView::TopToBottom, false);
    list->setItemDelegate(new DisplayItemDelegate(list));

    connect(list, &DListView::activated, this, [this](const QModelIndex &index) {
        emit this->selectIndex(index);
    });

    updateGeometry();
}

void PopupWidget::updateGeometry()
{
    Q_ASSERT(parentWidget());

    const int border = list->frameWidth();
    QSize size = parentWidget()->size();
    auto itemHeight = fontMetrics().height() * 2 + 5;   // same as itemdelegate
    auto height = 10 * itemHeight;
    if (model && model->rowCount() < 10)
        height = model->rowCount() * itemHeight;

    const QRect rect(parentWidget()->mapToGlobal(QPoint(-border, -(height + 5))), QSize(size.width(), height));
    setGeometry(rect);
}

void PopupWidget::selectFirstRow()
{
    if (model && (model->rowCount(QModelIndex()) > 0))
        list->setCurrentIndex(model->index(0, 0));
}

void PopupWidget::previous()
{
    int index = list->currentIndex().row();
    --index;
    if (index < 0) {
        // wrap
        index = model->rowCount(QModelIndex()) - 1;
    }
    list->setCurrentIndex(model->index(index, 0));
}

void PopupWidget::next()
{
    int index = list->currentIndex().row();
    ++index;
    if (index >= model->rowCount(QModelIndex())) {
        // wrap
        index = 0;
    }
    list->setCurrentIndex(model->index(index, 0));
}

bool PopupWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::LayoutRequest)
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

void PopupWidget::setmodel(ItemModel *model)
{
    list->setModel(model);
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
            emit selectIndex(list->currentIndex());
            return;
        }
        break;
    }
    DBlurEffectWidget::keyPressEvent(event);
}
