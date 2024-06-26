// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codecompletiondelegate.h"
#include "codecompletionmodel.h"
#include "codecompletionview.h"

#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#include <DPaletteHelper>
#endif

#include <QApplication>
#include <QTextLayout>
#include <QPainter>
#include <QDebug>
#include <QPainterPath>

DGUI_USE_NAMESPACE

CodeCompletionDelegate::CodeCompletionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QPixmap CodeCompletionDelegate::getIconPixmap(const QIcon &icon, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    auto px = icon.pixmap(size, mode, state);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    return px;
}

void CodeCompletionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    paintItemBackground(painter, opt, index);
    auto iconRect = paintItemIcon(painter, opt, index);

    opt.rect.adjust(iconRect.x() + 20, 0, 0, 0);
    opt.displayAlignment = Qt::AlignVCenter;

    // Set the text color to windowText color
    painter->setPen(option.palette.color(QPalette::WindowText));

    paintItemText(painter, opt, index);
}

QSize CodeCompletionDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(25);

    return size;
}

CodeCompletionView *CodeCompletionDelegate::view() const
{
    return qobject_cast<CodeCompletionView *>(parent());
}

void CodeCompletionDelegate::paintItemText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString text = index.data(CodeCompletionModel::NameRole).toString();
    // set formats
    QTextLayout textLayout(text, option.font, painter->device());

    // set alignment, rtls etc
    QTextOption textOption;
    textOption.setTextDirection(option.direction);
    textOption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
    textLayout.setTextOption(textOption);

    // layout the text
    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();
    if (!line.isValid())
        return;

    const int lineWidth = option.rect.width();
    line.setLineWidth(lineWidth);
    line.setPosition(QPointF(0, 0));

    textLayout.endLayout();

    int y = QStyle::alignedRect(Qt::LayoutDirectionAuto, option.displayAlignment, textLayout.boundingRect().size().toSize(), option.rect).y();

    // draw the text
    const auto pos = QPointF(option.rect.x(), y);
    textLayout.draw(painter, pos);
}

void CodeCompletionDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QRectF rect = option.rect;

    // draw background
    if (option.widget) {
#ifdef DTKWIDGET_CLASS_DPaletteHelper
        DPalette pl(DPaletteHelper::instance()->palette(option.widget));
#else
        const DPalette &pl = DGuiApplicationHelper::instance()->applicationPalette();
#endif

        QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
        QColor adjustColor = baseColor;

        bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
        if (isSelected) {
            // set highlight color
            QPalette::ColorGroup colorGroup = (option.widget ? option.widget->isEnabled() : (option.state & QStyle::State_Enabled))
                    ? QPalette::Normal
                    : QPalette::Disabled;

            if (colorGroup == QPalette::Normal && !(option.state & QStyle::State_Active))
                colorGroup = QPalette::Inactive;

            adjustColor = option.palette.color(colorGroup, QPalette::Highlight);
        } else if (option.state & QStyle::StateFlag::State_MouseOver) {
            // hover color
            adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        } else {
            // alternately background color
            painter->setOpacity(0);
            if (index.row() % 2 == 0) {
                adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +2);
                painter->setOpacity(1);
            }
        }

        // set paint path
        QPainterPath path;
        path.addRoundedRect(rect, 0, 0);

        // set render antialiasing
        painter->setRenderHints(QPainter::Antialiasing
                                | QPainter::TextAntialiasing
                                | QPainter::SmoothPixmapTransform);

        painter->fillPath(path, adjustColor);
    }

    painter->restore();
}

QRect CodeCompletionDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!view())
        return {};

    // draw icon
    QRect iconRect = option.rect;
    QSize iconSize = view()->iconSize();
    iconRect.setSize(iconSize);
    iconRect.moveLeft(iconRect.left() + 20);
    iconRect.moveTop(iconRect.top() + ((option.rect.bottom() - iconRect.bottom()) / 2));

    bool isEnabled = option.state & QStyle::State_Enabled;
    const QPixmap &px = getIconPixmap(index.data(CodeCompletionModel::IconRole).value<QIcon>(), iconRect.size(),
                                      isEnabled ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
    qreal x = iconRect.x();
    qreal y = iconRect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    y += (iconRect.size().height() - h) / 2.0;
    x += (iconRect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);

    return iconRect;
}
