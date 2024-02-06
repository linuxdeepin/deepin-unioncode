// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "pluginitemdelegate.h"
#include "pluginlistview.h"

#include <DPaletteHelper>
#include <DGuiApplicationHelper>

#include <QPainter>

inline constexpr int kRectRadius = { 0 }; // do not show rounded corners.
inline constexpr int kIconWidth = { 30 };
inline constexpr int kIconHeight = { 30 };
inline constexpr int kIconLeftMargin = { 10 };
inline constexpr int kTextLeftMargin = { 8 };

DWIDGET_USE_NAMESPACE
PluginItemDelegate::PluginItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate (parent)
{

}

void PluginItemDelegate::paint(QPainter *painter,
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
    QRectF iconRect = paintItemIcon(painter, opt, index);
    paintItemColumn(painter, opt, index, iconRect);

    painter->setOpacity(1);
}

QSize PluginItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    if (index.isValid()) {
        QSize size = index.data(Qt::SizeHintRole).toSize();
        if (size.isValid()) {
            return size;
        } else {
            return {option.rect.width(), option.fontMetrics.height() * 3 + 5};
        }
    }
    return DStyledItemDelegate::sizeHint(option, index);
}

void PluginItemDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    painter->save();

    if (option.widget) {
        DPalette pl(DPaletteHelper::instance()->palette(option.widget));
        QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
        QColor adjustColor = baseColor;

        bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;

        if (isSelected) {
            adjustColor = option.palette.color(DPalette::ColorGroup::Active, QPalette::Highlight);
        } else if (option.state & QStyle::StateFlag::State_MouseOver) {
            // hover color
            adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        } else {
            // alternately background color
            painter->setOpacity(0);
            if (index.row() % 2 == 0) {
                adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
                painter->setOpacity(1);
            }
        }

        // set paint path
        QPainterPath path;
        path.addRoundedRect(option.rect, kRectRadius, kRectRadius);
        painter->fillPath(path, adjustColor);
    }

    painter->restore();
}

QRectF PluginItemDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    Q_UNUSED(index);

    painter->save();

    if (!parent() || !parent()->parent())
        return QRect();


    // draw icon
    QRectF iconRect = option.rect;
    iconRect.setSize(QSize(kIconWidth, kIconHeight));
    iconRect.moveTopLeft(QPoint(kIconLeftMargin, qRound(iconRect.top() + (option.rect.bottom() - iconRect.bottom()) / 2)));

    // Copy of QStyle::alignedRect
    Qt::Alignment alignment = Qt::AlignCenter;
    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixelRatio = painter->device()->devicePixelRatioF();

    bool isEnabled = option.state & QStyle::State_Enabled;
    const QPixmap &px = getIconPixmap(option.icon, iconRect.size().toSize(), pixelRatio, isEnabled ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
    qreal x = iconRect.x();
    qreal y = iconRect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (iconRect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += iconRect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += iconRect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (iconRect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
    painter->restore();

    return iconRect;
}

QPixmap PluginItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio = 1.0, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    auto px = icon.pixmap(size, mode, state);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    return px;
}

void PluginItemDelegate::paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index, const QRectF &iconRect) const
{
    painter->save();

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected)
        painter->setPen(option.palette.color(DPalette::ColorGroup::Active, QPalette::HighlightedText));

    QFont boldFont = option.font;
    boldFont.setPointSize(10);
    boldFont.setBold(true);
    QFontMetrics fmBold(boldFont);

    // display plugin name.
    QString pluginName = index.data(Qt::DisplayRole).toString();
    pluginName = fmBold.elidedText(pluginName, Qt::ElideRight,
                               qRound(option.rect.width() - iconRect.width() - kIconLeftMargin * 2));
    painter->setFont(boldFont);
    painter->drawText(option.rect.adjusted(kIconLeftMargin + kIconWidth + kTextLeftMargin, 5, 0, -5),
                      Qt::AlignLeft | Qt::AlignTop, pluginName);
    painter->restore();

    painter->save();
    // display description.
    painter->setOpacity(0.7);
    QString description = index.data(PluginListView::Description).toString();
    QFontMetrics fm(option.font);
    description = fm.elidedText(description, Qt::ElideRight,
                                qRound(option.rect.width() - iconRect.width() - kIconLeftMargin * 2));
    painter->drawText(option.rect.adjusted(kIconLeftMargin + kIconWidth + kTextLeftMargin, 5, 0, -5),
                      Qt::AlignLeft | Qt::AlignVCenter, description);

    // display vender.
    boldFont.setPointSize(9);
    QFontMetrics fmSmallBold(boldFont);
    QString vender = index.data(PluginListView::Vender).toString();
    vender = fmSmallBold.elidedText(vender, Qt::ElideRight,
                           qRound(option.rect.width() - iconRect.width() - kIconLeftMargin * 2));
    painter->setFont(boldFont);
    painter->drawText(option.rect.adjusted(kIconLeftMargin + kIconWidth + kTextLeftMargin, 5, 0, -5),
                      Qt::AlignLeft | Qt::AlignBottom, vender);

    painter->restore();
}
