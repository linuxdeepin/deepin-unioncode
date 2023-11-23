// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displayitemdelegate.h"
#include "displayrecentview.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DStyle>
#include <DStyleOptionViewItem>
#include <DPaletteHelper>
#include <DSizeMode>
#include <DListView>

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QFileInfo>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextBlock>
#include <QAbstractItemView>

inline constexpr int kRectRadius = { 8 };
inline constexpr int kIconWidth = { 30 };
inline constexpr int kIconHeight = { 30 };
inline constexpr int kIconLeftMargin = { 10 };
inline constexpr int kTextLeftMargin = { 8 };

DisplayItemDelegate::DisplayItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate (parent)
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
    QRectF iconRect = paintItemIcon(painter, opt, index);
    paintItemColumn(painter, opt, index, iconRect);

    painter->setOpacity(1);
}

QSize DisplayItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    if (index.isValid()) {
        QSize size = index.data(Qt::SizeHintRole).toSize();
        if (size.isValid()) {
            return size;
        } else {
            return {option.rect.width(), option.fontMetrics.height() * 2 + 5};
        }
    }
    return DStyledItemDelegate::sizeHint(option, index);
}

void DisplayItemDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
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

QRectF DisplayItemDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    painter->save();

    if (!parent() || !parent()->parent())
        return QRect();

    bool isEnabled = option.state & QStyle::State_Enabled;

    // draw icon
    QRectF iconRect = option.rect;
    iconRect.setSize(QSize(kIconWidth, kIconHeight));
    iconRect.moveTopLeft(QPoint(kIconLeftMargin, qRound(iconRect.top() + (option.rect.bottom() - iconRect.bottom()) / 2)));

    // Copy of QStyle::alignedRect
    Qt::Alignment alignment = Qt::AlignCenter;
    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
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

QPixmap DisplayItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio = 1.0, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    // 确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    auto px = icon.pixmap(size, mode, state);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    return px;
}

void DisplayItemDelegate::paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index, const QRectF &iconRect) const
{
    painter->save();

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected)
        painter->setPen(option.palette.color(DPalette::ColorGroup::Active, QPalette::HighlightedText));

    QString filePath = index.data(Qt::DisplayRole).toString();
    QString fileName = QFileInfo(filePath).fileName();
    QFontMetrics fm(option.font);

    QString elidFilePath;
    elidFilePath = fm.elidedText(filePath, Qt::ElideMiddle,
                                 qRound(option.rect.width() - iconRect.width() - kIconLeftMargin * 2));

    painter->drawText(option.rect.adjusted(kIconLeftMargin + kIconWidth + kTextLeftMargin, 5, 0, -5),
                      Qt::AlignLeft | Qt::AlignBottom, elidFilePath);

    QString elidFileName;
    elidFileName = fm.elidedText(fileName, Qt::ElideMiddle,
                                 qRound(option.rect.width() - iconRect.width() - kIconLeftMargin * 2));
    painter->drawText(option.rect.adjusted(kIconLeftMargin + kIconWidth + kTextLeftMargin, 5, 0, -5),
                      Qt::AlignLeft | Qt::AlignTop, elidFileName);

    painter->restore();
}
