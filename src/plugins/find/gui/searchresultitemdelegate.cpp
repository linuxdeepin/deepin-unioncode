// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "constants.h"
#include "searchresultitemdelegate.h"
#include "searchresultmodel.h"

#include <DGuiApplicationHelper>
#include <DStyle>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QPainter>
#include <QTreeView>
#include <QToolTip>
#include <QApplication>

DWIDGET_USE_NAMESPACE

inline constexpr int Radius { 8 };
inline constexpr int Padding = { 6 };
inline constexpr int ItemMargin { 4 };
inline constexpr int ExpandArrowWidth { 20 };
inline constexpr int ExpandArrowHeight { 20 };
inline constexpr int ArrowAndIconDistance { 8 };
inline constexpr int SpacePadding { 8 };
inline constexpr int LineNumberWidth { 40 };
inline constexpr int OptionButtonSize { 20 };
inline constexpr int CountNumberSize { 20 };

SearchResultItemDelegate::SearchResultItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void SearchResultItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItem opt = option;
    DStyledItemDelegate::initStyleOption(&opt, index);
    painter->setRenderHint(QPainter::Antialiasing);

    drawBackground(painter, opt);
    if (!index.parent().isValid()) {
        const auto &iconRect = drawFileIcon(painter, opt, index);
        drawNameItem(painter, opt, index, iconRect);
    } else {
        drawContextItem(painter, option, index);
    }
}

QSize SearchResultItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return { 1000, 24 };
}

bool SearchResultItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        QString toolTip;
        QRect replaceRect = replaceButtonRect(option.rect);
        QRect closeRect = closeButtonRect(option.rect);
        if (replaceRect.contains(event->pos())) {
            toolTip = index.model()->hasChildren(index) ? tr("Replace All") : tr("Replace");
        } else if (closeRect.contains(event->pos())) {
            toolTip = tr("Dismiss");
        }

        if (!toolTip.isEmpty()) {
            QToolTip::showText(event->globalPos(), toolTip, view);
            return true;
        } else {
            QToolTip::hideText();
        }
    }

    return DStyledItemDelegate::helpEvent(event, view, option, index);
}

bool SearchResultItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && object == view()->viewport()) {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
        auto index = view()->indexAt(mouseEvent->pos());
        if (!index.isValid())
            return DStyledItemDelegate::eventFilter(object, event);

        auto itemRect = view()->visualRect(index);
        auto arrowRect = this->arrowRect(iconRect(itemRect));
        if (arrowRect.contains(mouseEvent->pos())) {
            if (view()->isExpanded(index)) {
                view()->collapse(index);
            } else {
                view()->expand(index);
            }
            return true;
        }

        auto replaceRect = replaceButtonRect(itemRect);
        if (replaceRect.contains(mouseEvent->pos())) {
            auto model = qobject_cast<SearchResultModel *>(view()->model());
            if (model) {
                Q_EMIT model->requestReplace(index);
                return true;
            }
        }

        auto closeRect = closeButtonRect(itemRect);
        if (closeRect.contains(mouseEvent->pos())) {
            auto model = qobject_cast<SearchResultModel *>(view()->model());
            if (model) {
                model->remove(index);
                return true;
            }
        }
    }

    return DStyledItemDelegate::eventFilter(object, event);
}

QTreeView *SearchResultItemDelegate::view() const
{
    return qobject_cast<QTreeView *>(parent());
}

void SearchResultItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();
    if (option.state.testFlag(QStyle::State_Selected)) {
        QColor bgColor = option.palette.color(QPalette::Normal, QPalette::Highlight);
        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, Radius, Radius);
    } else if (option.state.testFlag(QStyle::State_MouseOver)) {
#ifdef DTKWIDGET_CLASS_DPaletteHelper
        DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
        DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif
        painter->setBrush(palette.brush(DPalette::ItemBackground));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, Radius, Radius);
    }
    painter->restore();
}

QRect SearchResultItemDelegate::drawFileIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto iconRect = this->iconRect(option.rect);
    drawIcon(painter, option, option.icon, iconRect);
    drawExpandArrow(painter, iconRect, option, index);
    return iconRect;
}

QRect SearchResultItemDelegate::drawExpandArrow(QPainter *painter, const QRect &iconRect,
                                                const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!view())
        return {};

    QStyleOptionViewItem opt = option;
    opt.rect = arrowRect(iconRect).marginsRemoved(QMargins(5, 5, 5, 5));

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

QRect SearchResultItemDelegate::drawResultCount(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    painter->setFont(opt.font);
    auto resultCount = QString::number(index.model()->rowCount(index));
    auto countWidth = qMax(opt.fontMetrics.width(resultCount) + ItemMargin, CountNumberSize);

    QRect countRect = opt.rect;
    countRect.setSize({ countWidth, CountNumberSize });
    countRect.moveLeft(opt.rect.right() - countWidth - ItemMargin);
    countRect.moveTop(countRect.top() + ((opt.rect.bottom() - countRect.bottom()) / 2));

    painter->save();
#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif
    painter->setBrush(palette.brush(DPalette::ObviousBackground));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(countRect, Radius, Radius);

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (isSelected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    QTextOption textOption;
    textOption.setTextDirection(option.direction);
    textOption.setAlignment(QStyle::visualAlignment(option.direction, Qt::AlignCenter));
    painter->drawText(countRect, resultCount, textOption);
    painter->restore();
    return countRect;
}

QRect SearchResultItemDelegate::drawOptionButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;

    // draw close button
    QRect iconRect = closeButtonRect(opt.rect);
    QIcon closeIcon = DStyle::standardIcon(opt.widget->style(), DStyle::SP_CloseButton);
    drawIcon(painter, opt, closeIcon, iconRect);

    // draw replace button
    QIcon replaceIcon;
    if (!index.parent().isValid())
        replaceIcon = QIcon::fromTheme("replace_all");
    else
        replaceIcon = QIcon::fromTheme("replace");

    iconRect = replaceButtonRect(opt.rect);
    drawIcon(painter, opt, replaceIcon, iconRect);

    return iconRect;
}

void SearchResultItemDelegate::drawNameItem(QPainter *painter, const QStyleOptionViewItem &option,
                                            const QModelIndex &index, const QRect &iconRect) const
{
    QStyleOptionViewItem opt = option;
    painter->setFont(opt.font);

    // draw count
    QRect countRect;
    QRect optionRect;
    if (!opt.state.testFlag(QStyle::State_Selected) && !opt.state.testFlag(QStyle::State_MouseOver))
        countRect = drawResultCount(painter, opt, index);
    else
        optionRect = drawOptionButton(painter, opt, index);

#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif
    QRect nameRect = opt.rect;
    nameRect.setLeft(iconRect.right() + Padding);
    if (countRect.isValid())
        nameRect.setRight(countRect.left() - Padding);
    if (optionRect.isValid())
        nameRect.setRight(optionRect.left() - Padding);
    QString fileName = index.data(Qt::DisplayRole).toString();
    const auto &nameFormat = createFormatRange(opt, 0, fileName.length(),
                                               palette.color(QPalette::Normal, DPalette::BrightText), {});
    drawDisplay(painter, option, nameRect, fileName, { nameFormat });

    QString filePath = index.data(FilePathRole).toString();
    const auto &pathFormat = createFormatRange(opt, 0, filePath.length(),
                                               palette.color(QPalette::Normal, DPalette::TextTips), {});

    QRect pathRect = nameRect;
    pathRect.setLeft(nameRect.left() + opt.fontMetrics.width(fileName) + SpacePadding);
    drawDisplay(painter, option, pathRect, filePath, { pathFormat });
}

void SearchResultItemDelegate::drawContextItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    painter->setFont(opt.font);

    // draw option button
    QRect optionRect;
    if (opt.state.testFlag(QStyle::State_Selected) || opt.state.testFlag(QStyle::State_MouseOver))
        optionRect = drawOptionButton(painter, option, index);

    const auto &lineNumber = QString::number(index.data(LineRole).toInt());
    const auto &matchedText = index.data(MatchedTextRole).toString();
    const auto &column = index.data(ColumnRole).toInt();
    auto context = index.data(Qt::DisplayRole).toString();
    const auto &replaceText = index.data(ReplaceTextRole).toString();

#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif

    // draw line number
    const auto &lineNumbFormat = createFormatRange(opt, 0, lineNumber.length(),
                                                   palette.color(QPalette::Normal, DPalette::PlaceholderText), {});
    QRect lineNumberRect = opt.rect;
    lineNumberRect.setSize({ LineNumberWidth, lineNumberRect.height() });
    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    drawDisplay(painter, opt, lineNumberRect, lineNumber, { lineNumbFormat });

    // draw context
    QList<QTextLayout::FormatRange> formats;
    QRect textRect = opt.rect;
    textRect.setLeft(LineNumberWidth + SpacePadding);
    if (optionRect.isValid())
        textRect.setRight(optionRect.left() - Padding);
    if (!replaceText.isEmpty()) {
        int replaceTextOffset = column + matchedText.length();
        context.insert(replaceTextOffset, replaceText);
        QColor matchedBackground;
        QColor replaceBackground;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            matchedBackground.setNamedColor("#F9625C");
            matchedBackground.setAlpha(180);
            replaceBackground.setNamedColor("#BCDD75");
            replaceBackground.setAlpha(180);
        } else {
            matchedBackground.setNamedColor("#DB5C5C");
            matchedBackground.setAlpha(180);
            replaceBackground.setNamedColor("#57965C");
            replaceBackground.setAlpha(180);
        }
        formats << createFormatRange(opt, column, matchedText.length(), {}, matchedBackground);
        formats << createFormatRange(opt, replaceTextOffset, replaceText.length(), {}, replaceBackground);
    } else {
        QColor background;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            background.setNamedColor("#F3B517");
            background.setAlpha(180);
        } else {
            background.setNamedColor("#F2C55C");
            background.setAlpha(220);
        }
        formats << createFormatRange(opt, column, matchedText.length(), {}, background);
    }
    drawDisplay(painter, option, textRect, context, formats);
}

void SearchResultItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect,
                                           const QString &text, const QList<QTextLayout::FormatRange> &format) const
{
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Normal, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Normal, QPalette::Text));
    }

    if (text.isEmpty())
        return;

    const QStyleOptionViewItem opt = option;

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0);   // remove width padding
    const bool wrapText = opt.features & QStyleOptionViewItem::WrapText;
    QTextOption textOption;
    textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    textOption.setTextDirection(option.direction);
    textOption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
    QTextLayout textLayout;
    textLayout.setTextOption(textOption);
    textLayout.setFont(option.font);
    textLayout.setText(text);

    QSizeF textLayoutSize = doTextLayout(&textLayout, textRect.width());

    if (textRect.width() < textLayoutSize.width()
        || textRect.height() < textLayoutSize.height()) {
        QString elided;
        int start = 0;
        int end = text.indexOf(QChar::LineSeparator, start);
        if (end == -1) {
            elided += option.fontMetrics.elidedText(text, option.textElideMode, textRect.width());
        } else {
            while (end != -1) {
                elided += option.fontMetrics.elidedText(text.mid(start, end - start),
                                                        option.textElideMode, textRect.width());
                elided += QChar::LineSeparator;
                start = end + 1;
                end = text.indexOf(QChar::LineSeparator, start);
            }
            // let's add the last line (after the last QChar::LineSeparator)
            elided += option.fontMetrics.elidedText(text.mid(start),
                                                    option.textElideMode, textRect.width());
        }
        textLayout.setText(elided);
        textLayoutSize = doTextLayout(&textLayout, textRect.width());
    }

    const QSize layoutSize(textRect.width(), int(textLayoutSize.height()));
    const QRect layoutRect = QStyle::alignedRect(option.direction, option.displayAlignment,
                                                 layoutSize, textRect);

    textLayout.draw(painter, layoutRect.topLeft(), format.toVector(), layoutRect);
}

QSizeF SearchResultItemDelegate::doTextLayout(QTextLayout *textLayout, int width) const
{
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout->beginLayout();
    while (true) {
        QTextLine line = textLayout->createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(width);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    textLayout->endLayout();
    return QSizeF(widthUsed, height);
}

QRect SearchResultItemDelegate::iconRect(const QRect &itemRect) const
{
    QRect iconRect = itemRect;
    QSize iconSize = view()->iconSize();
    iconRect.setSize(iconSize);

    iconRect.moveLeft(iconRect.left() + ItemMargin + ExpandArrowWidth - ArrowAndIconDistance);
    iconRect.moveTop(iconRect.top() + ((itemRect.bottom() - iconRect.bottom()) / 2));

    return iconRect;
}

QRect SearchResultItemDelegate::arrowRect(const QRect &iconRect) const
{
    QRect arrowRect = iconRect;

    arrowRect.setSize(QSize(ExpandArrowWidth, ExpandArrowHeight));
    arrowRect.moveTop(iconRect.top() + (iconRect.bottom() - arrowRect.bottom()) / 2);
    arrowRect.moveCenter(QPoint(iconRect.left() - ArrowAndIconDistance, arrowRect.center().y()));

    return arrowRect;
}

QRect SearchResultItemDelegate::replaceButtonRect(const QRect &itemRect) const
{
    QRect replaceButtonRect = itemRect;

    replaceButtonRect.setSize(view()->iconSize());
    replaceButtonRect.moveLeft(itemRect.right() - 2 * view()->iconSize().width() - 2 * ItemMargin);
    replaceButtonRect.moveTop(replaceButtonRect.top() + ((itemRect.bottom() - replaceButtonRect.bottom()) / 2));

    return replaceButtonRect;
}

QRect SearchResultItemDelegate::closeButtonRect(const QRect &itemRect) const
{
    QRect closeButtonRect = itemRect;

    closeButtonRect.setSize(view()->iconSize());
    closeButtonRect.moveLeft(itemRect.right() - view()->iconSize().width() - ItemMargin);
    closeButtonRect.moveTop(closeButtonRect.top() + ((itemRect.bottom() - closeButtonRect.bottom()) / 2));

    return closeButtonRect;
}

QTextLayout::FormatRange SearchResultItemDelegate::createFormatRange(const QStyleOptionViewItem &option, int start, int length,
                                                                     const QColor &foreground, const QColor &background) const
{
    QTextLayout::FormatRange range;
    if (foreground.isValid() && !(option.state & QStyle::State_Selected))
        range.format.setForeground(foreground);
    if (background.isValid())
        range.format.setBackground(background);
    range.start = start;
    range.length = length;
    return range;
}

void SearchResultItemDelegate::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QIcon &icon, const QRect &rect) const
{
    QIcon::Mode iconMode = QIcon::Normal;
    if (!(option.state.testFlag(QStyle::State_Enabled)))
        iconMode = QIcon::Disabled;
    if (option.state.testFlag(QStyle::State_Selected))
        iconMode = QIcon::Selected;

    auto px = icon.pixmap({ 18, 18 }, iconMode);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    y += (rect.size().height() - h) / 2.0;
    x += (rect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}
