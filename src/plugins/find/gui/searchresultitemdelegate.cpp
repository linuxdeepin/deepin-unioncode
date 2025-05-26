// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
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
inline constexpr int ContentLeftSideMaxLength { 20 };
inline constexpr int ContentRightSideMaxLength { 100 };
inline constexpr char leftPadding[] { "…" };

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

    opt.rect.adjust(0, 0, -10, 0);
    painter->setRenderHint(QPainter::Antialiasing);
    drawBackground(painter, opt);
    if (!index.parent().isValid()) {
        const auto &iconRect = drawFileIcon(painter, opt, index);
        drawNameItem(painter, opt, index, iconRect);
    } else {
        drawContextItem(painter, opt, index);
    }
}

QSize SearchResultItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return { view()->width(), 24 };
}

bool SearchResultItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        auto opt = option;
        opt.rect.adjust(0, 0, -10, 0);
        QString toolTip;
        QRect replaceRect = replaceButtonRect(opt.rect);
        QRect closeRect = closeButtonRect(opt.rect);
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

bool SearchResultItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!index.isValid())
        return DStyledItemDelegate::editorEvent(event, model, option, index);

    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
        view()->update(index);
        break;
    case QEvent::MouseButtonRelease: {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
        auto itemRect = option.rect;
        itemRect.adjust(0, 0, -10, 0);
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
            auto viewModel = qobject_cast<SearchResultModel *>(model);
            if (viewModel) {
                Q_EMIT viewModel->requestReplace(index);
                return true;
            }
        }

        auto closeRect = closeButtonRect(itemRect);
        if (closeRect.contains(mouseEvent->pos())) {
            auto viewModel = qobject_cast<SearchResultModel *>(model);
            if (viewModel) {
                viewModel->remove(index);
                return true;
            }
        }
    } break;
    default:
        break;
    }

    return DStyledItemDelegate::editorEvent(event, model, option, index);
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
    auto countWidth = qMax(opt.fontMetrics.horizontalAdvance(resultCount) + ItemMargin, CountNumberSize);

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
    drawOptionBackground(painter, option, iconRect);
    drawIcon(painter, opt, QIcon::fromTheme("common_close"), iconRect);

    // draw replace button
    QIcon replaceIcon;
    if (!index.parent().isValid())
        replaceIcon = QIcon::fromTheme("replace_all");
    else
        replaceIcon = QIcon::fromTheme("replace");

    iconRect = replaceButtonRect(opt.rect);
    drawOptionBackground(painter, option, iconRect);
    drawIcon(painter, opt, replaceIcon, iconRect);
    return iconRect;
}

void SearchResultItemDelegate::drawOptionBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
    QPoint mousePos = view()->mapFromGlobal(QCursor::pos());
    if (!rect.contains(mousePos))
        return;

    if (QApplication::mouseButtons() & Qt::LeftButton) {
        QColor bgColor(255, 255, 255, qRound(255 * 0.15));
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect, 6, 6);
        painter->restore();
    } else if (option.state.testFlag(QStyle::State_MouseOver)) {
        QColor bgColor(0, 0, 0, qRound(255 * 0.08));
        if (option.state.testFlag(QStyle::State_Selected)) {
            bgColor.setRgba(qRgba(255, 255, 255, qRound(255 * 0.2)));
        } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            bgColor.setRgba(qRgba(255, 255, 255, qRound(255 * 0.08)));
        }
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect, 6, 6);
        painter->restore();
    }
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
    pathRect.setLeft(nameRect.left() + opt.fontMetrics.horizontalAdvance(fileName) + SpacePadding);
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
    const auto &matchedLength = index.data(MatchedLengthRole).toInt();
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
        int replaceTextOffset = column + matchedLength;
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
        auto matchedRange = createFormatRange(opt, column, matchedLength, {}, matchedBackground);
        matchedRange.format.setFontStrikeOut(true);
        formats << matchedRange;
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
        formats << createFormatRange(opt, column, matchedLength, {}, background);
    }

    auto [adjustedText, adjustedFormats] = adjustContent(index, context, formats);
    drawDisplay(painter, option, textRect, adjustedText, adjustedFormats);
}

void SearchResultItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect,
                                           const QString &text, const QList<QTextLayout::FormatRange> &formatList) const
{
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Normal, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Normal, QPalette::Text));
    }

    const QStyleOptionViewItem opt = option;

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0);   // remove width padding
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    textOption.setTextDirection(option.direction);
    textOption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
    QTextLayout textLayout;
    textLayout.setTextOption(textOption);
    textLayout.setFont(option.font);
    textLayout.setText(text);
    textLayout.setFormats(formatList.toVector());

    QSizeF textLayoutSize = doTextLayout(&textLayout, textRect.width());
    if (textRect.width() < textLayoutSize.width()) {
        QString displayText = option.fontMetrics.elidedText(text, Qt::ElideRight, textRect.width());
        textLayout.setText(displayText);
        doTextLayout(&textLayout, textRect.width());
    }

    textLayout.draw(painter, textRect.topLeft());
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

    replaceButtonRect.setSize({ 20, 20 });
    replaceButtonRect.moveLeft(itemRect.right() - 2 * 20 - 5);
    replaceButtonRect.moveTop(replaceButtonRect.top() + ((itemRect.bottom() - replaceButtonRect.bottom()) / 2));

    return replaceButtonRect;
}

QRect SearchResultItemDelegate::closeButtonRect(const QRect &itemRect) const
{
    QRect closeButtonRect = itemRect;

    closeButtonRect.setSize({ 20, 20 });
    closeButtonRect.moveLeft(itemRect.right() - 20 - 3);
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

    auto px = icon.pixmap(view()->iconSize(), iconMode);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    y += (rect.size().height() - h) / 2.0;
    x += (rect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}

QPair<QString, QList<QTextLayout::FormatRange>> SearchResultItemDelegate::adjustContent(const QModelIndex &index,
                                                                                        const QString &originalText,
                                                                                        const QList<QTextLayout::FormatRange> &formatList) const
{
    auto adjustFormatRange = [&](int offset) {
        if (offset == 0)
            return formatList;

        QList<QTextLayout::FormatRange> adjustedFormats;
        std::transform(formatList.cbegin(), formatList.cend(), std::back_inserter(adjustedFormats),
                       [&](const QTextLayout::FormatRange &format) {
                           return QTextLayout::FormatRange { format.start - offset, format.length, format.format };
                       });
        return adjustedFormats;
    };

    const auto &matchedLength = index.data(MatchedLengthRole).toInt();
    int keywordOffset = index.data(ColumnRole).toInt();
    const auto &replaceText = index.data(ReplaceTextRole).toString();

    auto displayText = originalText.trimmed();
    int offset = originalText.indexOf(displayText);
    keywordOffset -= offset;
    int replaceOffset = keywordOffset + matchedLength;

    int leftStart = std::max(0, keywordOffset - ContentLeftSideMaxLength);
    int rightEnd = std::min(displayText.length(), replaceOffset + replaceText.length() + ContentRightSideMaxLength);
    displayText = displayText.mid(leftStart, rightEnd - leftStart);
    if (leftStart != 0) {
        displayText.insert(0, leftPadding);
        leftStart -= 1;
    }

    return { displayText, adjustFormatRange(leftStart + offset) };
}
