// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectitemdelegate.h"
#include "projectitemmodel.h"
#include "utils/utils.h"
#include "common/itemnode.h"

#include <DGuiApplicationHelper>
#include <DStyle>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QToolTip>

inline constexpr int kPadding = { 6 };
inline constexpr int kItemMargin { 4 };
inline constexpr int kExpandArrowSize { 20 };
inline constexpr int kCheckBoxSize { 16 };
inline constexpr int kItemIndentation { 20 };
inline constexpr int kItemSpacing { 4 };

DWIDGET_USE_NAMESPACE

ProjectItemDelegate::ProjectItemDelegate(ProjectTreeView *parent)
    : DStyledItemDelegate(parent),
      view(parent)
{
}

ProjectItemDelegate::~ProjectItemDelegate()
{
    qDeleteAll(spinners);
    spinners.clear();
}

void ProjectItemDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItem opt = option;
    DStyledItemDelegate::initStyleOption(&opt, index);

    opt.rect.adjust(10, 0, -10, 0);
    painter->setRenderHint(QPainter::Antialiasing);
    drawBackground(painter, opt);
    int depth = itemDepth(index);
    const auto &iconRect = drawFileIcon(depth, painter, opt, index);
    drawFileNameItem(painter, opt, index, iconRect);
}

QSize ProjectItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    return QSize(view->width(), 24);
}

bool ProjectItemDelegate::editorEvent(QEvent *event,
                                      QAbstractItemModel *model,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
        QStyleOptionViewItem opt = option;
        opt.rect.adjust(10, 0, -10, 0);
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        int depth = itemDepth(index);
        if (index.model()->flags(index).testFlag(Qt::ItemIsUserCheckable)) {
            const QRect checkRect = checkBoxRect(depth, opt.rect);
            if (checkRect.contains(mouseEvent->pos())) {
                Qt::CheckState state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
                Qt::CheckState newState = state == Qt::Checked ? Qt::Unchecked : Qt::Checked;

                updateChildrenCheckState(model, index, newState);
                updateParentCheckState(model, index);
                return true;
            }
        }

        const QRect arRect = arrowRect(depth, opt.rect);
        if (arRect.contains(mouseEvent->pos())) {
            if (view->isExpanded(index)) {
                view->collapse(index);
            } else {
                view->expand(index);
            }
            return true;
        }
    }

    return false;
}

bool ProjectItemDelegate::helpEvent(QHelpEvent *event,
                                    QAbstractItemView *view,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        auto opt = option;
        opt.rect.adjust(10, 0, -10, 0);
        QRect stateRect = itemStateRect(opt.rect);
        if (index.data(ItemStateRole) == Failed && stateRect.contains(event->pos())) {
            auto model = qobject_cast<ProjectItemModel *>(view->model());
            auto item = model->itemForIndex(index);
            QToolTip::showText(event->globalPos(), item->userCache, view);
            return true;
        }
    }

    return DStyledItemDelegate::helpEvent(event, view, option, index);
}

void ProjectItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();
    if (option.state.testFlag(QStyle::State_Selected)) {
        QColor bgColor = option.palette.color(QPalette::Normal, QPalette::Highlight);
        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 8, 8);
    } else if (option.state.testFlag(QStyle::State_MouseOver)) {
#ifdef DTKWIDGET_CLASS_DPaletteHelper
        DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
        DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif
        painter->setBrush(palette.brush(DPalette::ItemBackground));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 8, 8);
    }
    painter->restore();
}

QRect ProjectItemDelegate::drawFileIcon(int depth,
                                        QPainter *painter,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    auto iconRect = fileIconRect(depth, option.rect, index);
    drawIcon(painter, option, option.icon, iconRect);
    if (index.model()->flags(index).testFlag(Qt::ItemIsUserCheckable))
        drawCheckBox(depth, painter, option, index);
    if (index.model()->hasChildren(index))
        drawExpandArrow(depth, painter, option, index);
    return iconRect;
}

QRect ProjectItemDelegate::drawExpandArrow(int depth, QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.rect = arrowRect(depth, opt.rect).marginsRemoved(QMargins(5, 5, 5, 5));

    painter->save();
    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    auto style = option.widget->style();
    if (view->isExpanded(index)) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, nullptr);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter, nullptr);
    }

    painter->restore();
    return opt.rect;
}

void ProjectItemDelegate::drawCheckBox(int depth,
                                       QPainter *painter,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    QStyleOptionButton opt;
    opt.rect = checkBoxRect(depth, option.rect);
    opt.state = option.state;

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected) {
        opt.palette.setColor(QPalette::WindowText, option.palette.color(QPalette::HighlightedText));
        opt.palette.setColor(QPalette::Highlight, option.palette.color(QPalette::HighlightedText));
    }

    Qt::CheckState checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
    if (checkState == Qt::Checked)
        opt.state |= QStyle::State_On;
    else if (checkState == Qt::PartiallyChecked)
        opt.state |= QStyle::State_NoChange;
    else
        opt.state |= QStyle::State_Off;

    // Draw the checkbox using the widget's style
    option.widget->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, painter, option.widget);
}

QRect ProjectItemDelegate::drawItemState(QPainter *painter,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    if (index.model()->hasChildren(index))
        return {};

    auto stopSpinner = [&] {
        if (auto spinner = spinners.value(index)) {
            delete spinner;
            spinners.remove(index);
        }
    };

    QRect stateRect = itemStateRect(option.rect);
    auto state = static_cast<ItemState>(index.data(ItemStateRole).toInt());
    switch (state) {
    case Generating: {
        auto *spinner = findOrCreateSpinnerPainter(index);
        QColor color = option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText)
                                                             : option.palette.color(QPalette::Highlight);

        spinner->paint(*painter, color, stateRect);
        return stateRect;
    }
    case Waiting:
        stopSpinner();
        drawIcon(painter, option, QIcon::fromTheme("uc_wait"), stateRect);
        return stateRect;
    case Completed:
        stopSpinner();
        drawIcon(painter, option, QIcon::fromTheme("uc_success"), stateRect);
        return stateRect;
    case Failed:
        stopSpinner();
        drawIcon(painter, option, QIcon::fromTheme("uc_failure"), stateRect);
        return stateRect;
    case Ignored:
        stopSpinner();
        drawIcon(painter, option, QIcon::fromTheme("uc_ignore"), stateRect);
        return stateRect;
    case None:
    default:
        break;
    }

    stopSpinner();
    return {};
}

void ProjectItemDelegate::drawFileNameItem(QPainter *painter,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index,
                                           const QRect &iconRect) const
{
    QStyleOptionViewItem opt = option;
    painter->setFont(opt.font);

    QRect stateRect;
    if (view->viewType() == ProjectTreeView::UnitTest)
        stateRect = drawItemState(painter, opt, index);

    QRect nameRect = opt.rect;
    nameRect.setLeft(iconRect.right() + kPadding);
    if (stateRect.isValid())
        nameRect.setRight(stateRect.left() - kPadding);

    QString fileName = index.data(Qt::DisplayRole).toString();
    if (opt.state & QStyle::State_Selected) {
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::HighlightedText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
    }

    QString displayText = opt.fontMetrics.elidedText(fileName, Qt::ElideRight, nameRect.width());
    painter->drawText(nameRect, displayText);
}

void ProjectItemDelegate::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QIcon &icon, const QRect &rect) const
{
    QIcon::Mode iconMode = QIcon::Normal;
    if (!(option.state.testFlag(QStyle::State_Enabled)))
        iconMode = QIcon::Disabled;
    if (option.state.testFlag(QStyle::State_Selected))
        iconMode = QIcon::Selected;

    auto px = icon.pixmap(view->iconSize(), iconMode);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    y += (rect.size().height() - h) / 2.0;
    x += (rect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}

QRect ProjectItemDelegate::checkBoxRect(int depth, const QRect &itemRect) const
{
    QRect checkRect = itemRect;
    checkRect.adjust(depth * kItemIndentation, 0, 0, 0);
    checkRect.setSize({ kCheckBoxSize, kCheckBoxSize });

    const auto &ar = arrowRect(depth, itemRect);
    checkRect.moveLeft(ar.right());
    checkRect.moveTop(checkRect.top() + ((itemRect.bottom() - checkRect.bottom()) / 2));

    return checkRect;
}

QRect ProjectItemDelegate::fileIconRect(int depth, const QRect &itemRect, const QModelIndex &index) const
{
    QRect iconRect = itemRect;
    iconRect.adjust(depth * kItemIndentation, 0, 0, 0);
    QSize iconSize = view->iconSize();
    iconRect.setSize(iconSize);

    if (index.flags().testFlag(Qt::ItemIsUserCheckable)) {
        const auto &checkRect = checkBoxRect(depth, itemRect);
        iconRect.moveLeft(checkRect.right() + kItemSpacing);
    } else {
        const auto &ar = arrowRect(depth, itemRect);
        iconRect.moveLeft(ar.right() + kItemSpacing);
    }
    iconRect.moveTop(iconRect.top() + ((itemRect.bottom() - iconRect.bottom()) / 2));

    return iconRect;
}

QRect ProjectItemDelegate::arrowRect(int depth, const QRect &itemRect) const
{
    QRect arrowRect = itemRect;
    arrowRect.adjust(depth * kItemIndentation, 0, 0, 0);

    arrowRect.setSize(QSize(kExpandArrowSize, kExpandArrowSize));
    arrowRect.moveLeft(arrowRect.left() + kItemMargin);
    arrowRect.moveTop(itemRect.top() + (itemRect.bottom() - arrowRect.bottom()) / 2);

    return arrowRect;
}

QRect ProjectItemDelegate::itemStateRect(const QRect &itemRect) const
{
    QRect stateRect = itemRect;
    stateRect.setSize(view->iconSize());

    stateRect.moveLeft(itemRect.right() - stateRect.width() - 10);
    stateRect.moveTop(itemRect.top() + (itemRect.bottom() - stateRect.bottom()) / 2);

    return stateRect;
}

int ProjectItemDelegate::itemDepth(const QModelIndex &index) const
{
    int depth = 0;
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        depth++;
        parent = parent.parent();
    }
    return depth;
}

SpinnerPainter *ProjectItemDelegate::findOrCreateSpinnerPainter(const QModelIndex &index) const
{
    SpinnerPainter *sp = spinners.value(index);
    if (!sp) {
        sp = new SpinnerPainter();
        sp->setUpdateCallback([index, this] { view->update(index); });
        sp->startAnimation();
        spinners.insert(index, sp);
    }
    return sp;
}

void ProjectItemDelegate::updateChildrenCheckState(QAbstractItemModel *model,
                                                   const QModelIndex &index,
                                                   Qt::CheckState state)
{
    if (!index.isValid())
        return;

    // Update current node
    model->setData(index, state, Qt::CheckStateRole);

    // Recursively update all children
    if (model->hasChildren(index)) {
        for (int i = 0; i < model->rowCount(index); ++i) {
            QModelIndex child = model->index(i, 0, index);
            updateChildrenCheckState(model, child, state);
        }
    }
}

void ProjectItemDelegate::updateParentCheckState(QAbstractItemModel *model, const QModelIndex &index)
{
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        bool allChecked = true;
        bool anyChecked = false;

        for (int i = 0; i < model->rowCount(parent); ++i) {
            QModelIndex child = model->index(i, 0, parent);
            Qt::CheckState childState = static_cast<Qt::CheckState>(child.data(Qt::CheckStateRole).toInt());

            if (childState != Qt::Checked)
                allChecked = false;
            if (childState == Qt::Checked || childState == Qt::PartiallyChecked)
                anyChecked = true;
        }

        Qt::CheckState parentState = allChecked ? Qt::Checked : anyChecked ? Qt::PartiallyChecked : Qt::Unchecked;
        model->setData(parent, parentState, Qt::CheckStateRole);
        parent = parent.parent();
    }
}
