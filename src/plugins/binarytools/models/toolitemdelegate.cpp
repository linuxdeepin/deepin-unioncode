// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolitemdelegate.h"

#include <DGuiApplicationHelper>
#include <DStyle>
#include <DLineEdit>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QPainter>
#include <QTreeView>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

ToolItemDelegate::ToolItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void ToolItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItem opt = option;
    DStyledItemDelegate::initStyleOption(&opt, index);

    painter->setRenderHint(QPainter::Antialiasing);
    drawBackground(painter, opt);
    drawToolItem(painter, opt, index);
}

QSize ToolItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return { view()->width(), 24 };
}

QWidget *ToolItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    auto edit = new DLineEdit(parent);
    edit->setAutoFillBackground(true);
    edit->setBackgroundRole(QPalette::Base);
    return edit;
}

void ToolItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto edit = qobject_cast<DLineEdit *>(editor);
    edit->setText(index.data(Qt::DisplayRole).toString());
}

void ToolItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto edit = qobject_cast<DLineEdit *>(editor);
    const auto &text = edit->text();
    if (!text.isEmpty())
        model->setData(index, text);
}

void ToolItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (view()) {
        QRect rect = view()->visualRect(index);
        editor->setGeometry(rect);
    } else {
        DStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
}

bool ToolItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        const QRect arRect = arrowRect(option.rect);
        if (arRect.contains(mouseEvent->pos())) {
            if (view()->isExpanded(index)) {
                view()->collapse(index);
            } else {
                view()->expand(index);
            }
            return true;
        }
    }

    return false;
}

QTreeView *ToolItemDelegate::view() const
{
    return qobject_cast<QTreeView *>(parent());
}

void ToolItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();
    if (option.state.testFlag(QStyle::State_Selected)) {
        QColor bgColor = option.palette.color(QPalette::Normal, QPalette::Highlight);
        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 6, 6);
    } else if (option.state.testFlag(QStyle::State_MouseOver)) {
#ifdef DTKWIDGET_CLASS_DPaletteHelper
        DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
        DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif
        painter->setBrush(palette.brush(DPalette::ItemBackground));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 6, 6);
    }
    painter->restore();
}

void ToolItemDelegate::drawToolItem(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QRect extraRect;
    if (!index.parent().isValid())
        extraRect = drawExpandArrow(painter, option, index);
    else
        extraRect = drawItemIcon(painter, option, index);

    QRect itemRect = option.rect;
    itemRect.setLeft(extraRect.right() + 6);

    QString toolName = index.data(Qt::DisplayRole).toString();
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Normal, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Normal, QPalette::Text));
    }

    QString displayText = option.fontMetrics.elidedText(toolName, Qt::ElideRight, itemRect.width());
    painter->drawText(itemRect, displayText);
}

QRect ToolItemDelegate::drawItemIcon(QPainter *painter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    QRect iconRect = option.rect;
    iconRect.setSize(view()->iconSize());
    iconRect.moveLeft(iconRect.left() + 24);
    iconRect.moveTop(option.rect.top() + (option.rect.bottom() - iconRect.bottom()) / 2);

    QIcon::Mode iconMode = QIcon::Normal;
    if (!(option.state.testFlag(QStyle::State_Enabled)))
        iconMode = QIcon::Disabled;
    if (option.state.testFlag(QStyle::State_Selected))
        iconMode = QIcon::Selected;

    auto px = option.icon.pixmap(view()->iconSize(), iconMode);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    qreal x = iconRect.x();
    qreal y = iconRect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    y += (iconRect.size().height() - h) / 2.0;
    x += (iconRect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
    return iconRect;
}

QRect ToolItemDelegate::drawExpandArrow(QPainter *painter,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.rect = arrowRect(opt.rect).marginsRemoved(QMargins(5, 5, 5, 5));

    painter->save();
    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    auto style = option.widget->style();
    if (view()->isExpanded(index)) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, nullptr);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter, nullptr);
    }

    painter->restore();
    return opt.rect;
}

QRect ToolItemDelegate::arrowRect(const QRect &itemRect) const
{
    QRect arrowRect = itemRect;

    arrowRect.setSize(QSize(20, 20));
    arrowRect.moveLeft(arrowRect.left() + 4);
    arrowRect.moveTop(itemRect.top() + (itemRect.bottom() - arrowRect.bottom()) / 2);

    return arrowRect;
}
