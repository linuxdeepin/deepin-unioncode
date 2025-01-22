// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegate.h"
#include "notificationlistview.h"

#include <DStyle>
#include <DFrame>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DStyle>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyleOptionButton>

DWIDGET_USE_NAMESPACE

constexpr int kRadius { 8 };
constexpr int kNotificationSourceHeight { 15 };
constexpr int kNotificationActionHeight { 32 };
constexpr int kNotificationSpacing { 10 };

ItemDelegate::ItemDelegate(NotificationListView *view, QObject *parent)
    : QStyledItemDelegate(parent),
      view(view)
{
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return QStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItem opt = option;
    QStyledItemDelegate::initStyleOption(&opt, index);

    painter->setRenderHint(QPainter::Antialiasing);
    drawBackground(painter, opt);
    auto iconRect = drawIcon(painter, opt, index);
    drawDisplayText(painter, opt, index, iconRect);
    drawActionButton(painter, option, index);
}

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) {
        Q_EMIT model->layoutChanged({ QPersistentModelIndex(index) });
        return false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            auto entity = index.data(kEntityRole).value<EntityPtr>();
            QPoint mousePos = view->mapFromGlobal(QCursor::pos());
            const auto &closeRect = closeButtonRect(option.rect);
            if (closeRect.contains(mousePos)) {
                Q_EMIT view->processed(entity);
                return true;
            }

            const auto &actInfoList = actionInfoList(option, index);
            for (const auto &info : actInfoList) {
                if (info.rect.contains(mousePos)) {
                    Q_EMIT view->actionInvoked(entity, info.id);
                    Q_EMIT view->processed(entity);
                    return true;
                }
            }
        }
    }

    return false;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int height = kNotificationSpacing * 2;
    QString message = index.data(Qt::DisplayRole).toString();
    QFontMetrics fm(option.font);
    int fontLeading = fm.leading();
    // Layout the message
    int textWidth = closeButtonRect(option.rect).left()
            - view->iconSize().width() - kNotificationSpacing;

    message.replace(QLatin1Char('\n'), QChar::LineSeparator);
    QTextLayout tl(message);
    tl.beginLayout();
    while (true) {
        QTextLine line = tl.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(textWidth);
        height += fontLeading;
        line.setPosition(QPoint(0, height));
        height += static_cast<int>(line.height());
    }
    tl.endLayout();

    QString source = index.data(kSourceRole).toString();
    if (!source.isEmpty()) {
        height += kNotificationSourceHeight + kNotificationSpacing;
    }

    auto actList = index.data(kActionsRole).toStringList();
    if (!actList.isEmpty()) {
        height += kNotificationActionHeight;
    }

    return { option.rect.width(), height };
}

void ItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();
#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif
    painter->setBrush(palette.brush(DPalette::ItemBackground));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(option.rect, kRadius, kRadius);
    painter->restore();
}

QRect ItemDelegate::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect = option.rect;
    rect.setSize(view->iconSize());
    rect.moveLeft(rect.left() + kNotificationSpacing);
    rect.moveTop(rect.top() + kNotificationSpacing);

    auto px = option.icon.pixmap(view->iconSize());
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    painter->drawPixmap(rect, px);
    return rect;
}

QRect ItemDelegate::drawDisplayText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &iconRect) const
{
    QRect textRect = drawNotificationText(painter, option, index, iconRect);
    textRect = drawSourceText(painter, option, index, textRect);
    return textRect;
}

QRect ItemDelegate::drawNotificationText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rect) const
{
    QRect textRect = option.rect;
    textRect.setLeft(rect.right() + kNotificationSpacing);
    textRect.moveTop(textRect.top() + kNotificationSpacing - 4);
    const auto &btnRect = drawCloseButton(painter, option);
    textRect.setRight(btnRect.left() - kNotificationSpacing);

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WordWrap);
    textOption.setTextDirection(option.direction);
    textOption.setAlignment(QStyle::visualAlignment(option.direction, Qt::AlignLeft | Qt::AlignVCenter));

    QTextLayout textLayout;
    textLayout.setTextOption(textOption);
    textLayout.setFont(option.font);
    textLayout.setText(option.text);

    QSizeF textLayoutSize = doTextLayout(&textLayout, textRect.width());
    textRect.setSize(textLayoutSize.toSize());
    textLayout.draw(painter, textRect.topLeft());
    return textRect;
}

QRect ItemDelegate::drawSourceText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rect) const
{
    QString sourceText = index.data(kSourceRole).toString();
    if (sourceText.isEmpty())
        return rect;

    QRect textRect = option.rect;
    textRect.setLeft(rect.left());
    textRect.moveTop(rect.bottom() + kNotificationSpacing - 4);

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    textOption.setTextDirection(option.direction);
    textOption.setAlignment(QStyle::visualAlignment(option.direction, Qt::AlignLeft | Qt::AlignVCenter));

    QTextLayout textLayout;
    textLayout.setTextOption(textOption);
    textLayout.setFont(DFontSizeManager::instance()->get(DFontSizeManager::T7, option.font));
    textLayout.setText(sourceText);

    QSizeF textLayoutSize = doTextLayout(&textLayout, textRect.width());
    if (textRect.width() < textLayoutSize.width()) {
        QString displayText = option.fontMetrics.elidedText(sourceText, Qt::ElideRight, textRect.width());
        textLayout.setText(displayText);
        textLayoutSize = doTextLayout(&textLayout, textRect.width());
    }

#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette palette = DPaletteHelper::instance()->palette(option.widget);
#else
    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
#endif
    painter->save();
    painter->setPen(palette.color(DPalette::TextTips));
    textLayout.draw(painter, textRect.topLeft());
    painter->restore();

    return textRect;
}

QRect ItemDelegate::drawCloseButton(QPainter *painter, const QStyleOptionViewItem &option) const
{
    const auto &rect = closeButtonRect(option.rect);
    if (option.state & QStyle::State_MouseOver) {
        DStyleOptionButton opt;
        opt.icon = DStyle::standardIcon(option.widget->style(), DStyle::SP_CloseButton);
        opt.iconSize = rect.size();
        opt.rect = rect;
        opt.state = QStyle::State_Enabled;
        opt.features |= DStyleOptionButton::Flat;
        opt.init(option.widget);

        QPoint mousePos = view->mapFromGlobal(QCursor::pos());
        bool isHovered = rect.contains(mousePos);
        if (option.state & QStyle::State_MouseOver && isHovered) {
            opt.state |= QStyle::State_MouseOver;
            // pressed
            if (QApplication::mouseButtons() & Qt::LeftButton)
                opt.state |= QStyle::State_Sunken;
        }

        painter->save();
        DStyle::drawControl(option.widget->style(), DStyle::CE_IconButton, &opt, painter, option.widget);
        painter->restore();
    }

    return rect;
}

void ItemDelegate::drawActionButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto &actInfoList = actionInfoList(option, index);
    for (const auto &info : actInfoList) {
        DStyleOptionButton opt;
        opt.text = info.text;
        opt.rect = info.rect;
        opt.state = QStyle::State_Enabled;
        opt.init(option.widget);

        QPoint mousePos = view->mapFromGlobal(QCursor::pos());
        bool isHovered = info.rect.contains(mousePos);
        if (option.state & QStyle::State_MouseOver && isHovered) {
            opt.state |= QStyle::State_MouseOver;
            // pressed
            if (QApplication::mouseButtons() & Qt::LeftButton)
                opt.state |= QStyle::State_Sunken;
        }

        painter->save();
        if (info.id.endsWith(DefaultButtonField)) {
            opt.features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::SuggestButton);
            QColor startColor = option.palette.color(QPalette::Highlight);
            QColor endColor = DGuiApplicationHelper::adjustColor(startColor, 0, 0, +10, 0, 0, 0, 0);

            opt.palette.setBrush(QPalette::Light, QBrush(endColor));
            opt.palette.setBrush(QPalette::Dark, QBrush(startColor));
            opt.palette.setBrush(QPalette::ButtonText, option.palette.highlightedText());
        }

        option.widget->style()->drawControl(QStyle::CE_PushButton, &opt, painter, option.widget);
        painter->restore();
    }
}

QSizeF ItemDelegate::doTextLayout(QTextLayout *textLayout, int width) const
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

QList<ActionBtuuonInfo> ItemDelegate::actionInfoList(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto actList = index.data(kActionsRole).toStringList();
    if (actList.isEmpty())
        return {};

    QList<ActionBtuuonInfo> actInfoList;
    ActionBtuuonInfo info;
    QRect tmpRect = option.rect;
    for (int i = actList.size() - 1; i >= 0; --i) {
        if (i % 2 == 0) {
            info.id = actList[i];
            actInfoList.append(info);
        } else {
            info.text = actList[i];
            info.rect = tmpRect;
            auto textRect = option.fontMetrics.boundingRect(0, 0, tmpRect.width(), 0, Qt::TextSingleLine, info.text);
            textRect.adjust(-15, -5, 15, 5);
            info.rect.setSize(textRect.size());
            info.rect.moveRight(tmpRect.right() - kNotificationSpacing);
            info.rect.moveBottom(option.rect.bottom() - kNotificationSpacing);
            tmpRect.moveRight(info.rect.left());
        }
    }

    return actInfoList;
}

QRect ItemDelegate::closeButtonRect(const QRect &itemRect) const
{
    QRect rect = itemRect;
    rect.setSize({ 24, 24 });
    rect.moveRight(itemRect.right() - kNotificationSpacing);
    rect.moveTop(itemRect.top() + kNotificationSpacing - 4);
    return rect;
}
