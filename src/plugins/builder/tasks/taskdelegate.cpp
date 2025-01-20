// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskdelegate.h"
#include "taskmodel.h"

#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#include <DPaletteHelper>
#endif

#include <QTextLayout>
#include <QPainter>
#include <QDir>
#include <QPainterPath>
#include <QPalette>
#include <QApplication>

DWIDGET_USE_NAMESPACE

inline constexpr int kRectRadius { 8 };
inline constexpr int kTaskIconSize { 16 };
inline constexpr int kItemMargin { 10 };
inline constexpr int kItemMinHeight { 24 };

TaskDelegate::TaskDelegate(TaskView *parent)
    : DStyledItemDelegate(parent),
      view(parent)
{
}

TaskDelegate::~TaskDelegate() = default;

QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.rect.adjust(8, 0, -8, 0);

    auto view = qobject_cast<const QAbstractItemView *>(opt.widget);
    const bool selected = (view->selectionModel()->currentIndex() == index);
    QSize s;
    s.setWidth(opt.rect.width());

    if (selected) {
        QFontMetrics fm(option.font);
        int fontHeight = fm.height();
        int fontLeading = fm.leading();
        QString description = index.data(TaskModel::Description).toString();
        // Layout the description
        int leading = fontLeading;
        int height = 0;
        int textWidth = fixButtonRect(opt.rect).left()
                - iconRect(opt.rect).right()
                - 2 * kItemMargin + 1;

        description.replace(QLatin1Char('\n'), QChar::LineSeparator);
        QTextLayout tl(description);
        tl.beginLayout();
        while (true) {
            QTextLine line = tl.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(textWidth);
            height += leading;
            line.setPosition(QPoint(0, height));
            height += static_cast<int>(line.height());
        }
        tl.endLayout();

        s.setHeight(height + leading + fontHeight + 3);
    }

    if (s.height() < kItemMinHeight)
        s.setHeight(kItemMinHeight);

    return s;
}

bool TaskDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) {
        Q_EMIT model->layoutChanged({ QPersistentModelIndex(index) });
        return false;
    } else if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QPoint mousePos = option.widget->mapFromGlobal(QCursor::pos());
            auto itemRect = option.rect.adjusted(8, 0, -8, 0);
            const auto &btnRect = fixButtonRect(itemRect);
            if (btnRect.contains(mousePos)) {
                Q_EMIT model->layoutChanged({ QPersistentModelIndex(index) });
                Q_EMIT view->sigFixIssue(index);
                return true;
            }
        }
    }

    return false;
}

void TaskDelegate::emitSizeHintChanged(const QModelIndex &index)
{
    emit sizeHintChanged(index);
}

void TaskDelegate::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit sizeHintChanged(current);
    emit sizeHintChanged(previous);
}

void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);

    painter->setRenderHints(painter->renderHints()
                            | QPainter::Antialiasing
                            | QPainter::TextAntialiasing
                            | QPainter::SmoothPixmapTransform);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.rect.adjust(8, 0, -8, 0);
    paintItemBackground(painter, opt, index);
    painter->setOpacity(1);

    QIcon icon = index.data(TaskModel::Icon).value<QIcon>();
    QRect icRect = iconRect(opt.rect);
    painter->drawPixmap(icRect, icon.pixmap(kTaskIconSize, kTaskIconSize));
    paintItemColumn(painter, opt, index, icRect);
}

void TaskDelegate::paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
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

void TaskDelegate::paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index, const QRect &iconRect) const
{
    auto btnRect = paintFixButton(painter, option, index);
    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;

    painter->save();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType) {
        painter->setPen(QColor(255, 255, 255, 180));
    } else {
        painter->setPen(QColor(0, 0, 0, 180));
    }

    QRect textRect = option.rect;
    textRect.setLeft(iconRect.right() + kItemMargin);
    textRect.setRight(btnRect.left() - kItemMargin);

    QString description = index.data(TaskModel::Description).toString();
    if (!isSelected) {
        description = option.fontMetrics.elidedText(description, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignLeft, description);
    } else {
        QFontMetrics fm(option.font);
        description.replace(QLatin1Char('\n'), QChar::LineSeparator);
        painter->setPen(QColor(Qt::white));
        int height = 0;
        int leading = fm.leading();
        QTextLayout tl(description);
        tl.beginLayout();
        while (true) {
            QTextLine line = tl.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(textRect.width());
            height += leading;
            line.setPosition(QPoint(0, height));
            height += static_cast<int>(line.height());
        }
        tl.endLayout();

        tl.draw(painter, textRect.topLeft());
    }

    QString directory = QDir::toNativeSeparators(index.data(TaskModel::File).toString());
    if (isSelected) {
        painter->setPen(QColor(Qt::white));
        if (index.data(TaskModel::FileNotFound).toBool() && !directory.isEmpty()) {
            QString fileNotFound = tr("File not found: %1").arg(directory);
            fileNotFound = option.fontMetrics.elidedText(fileNotFound, Qt::ElideRight, textRect.width());
            painter->setPen(Qt::red);
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignBottom, fileNotFound);
        } else {
            directory = option.fontMetrics.elidedText(directory, Qt::ElideRight, textRect.width());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignBottom, directory);
        }
    }

    painter->restore();
}

QRect TaskDelegate::paintFixButton(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    auto btnRect = fixButtonRect(option.rect);
    if (!option.state.testFlag(QStyle::State_MouseOver))
        return btnRect;

    QPoint mousePos = view->mapFromGlobal(QCursor::pos());
    if (btnRect.contains(mousePos)) {
        if (QApplication::mouseButtons() & Qt::LeftButton) {
            QColor bgColor(255, 255, 255, qRound(255 * 0.15));
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColor);
            painter->drawRoundedRect(btnRect, 6, 6);
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
            painter->drawRoundedRect(btnRect, 6, 6);
            painter->restore();
        }
    }

    QIcon::Mode iconMode = QIcon::Normal;
    if (!(option.state.testFlag(QStyle::State_Enabled)))
        iconMode = QIcon::Disabled;
    if (option.state.testFlag(QStyle::State_Selected))
        iconMode = QIcon::Selected;

    auto icon = QIcon::fromTheme("uc_repair");
    auto px = icon.pixmap({ kTaskIconSize, kTaskIconSize }, iconMode);
    px.setDevicePixelRatio(qApp->devicePixelRatio());
    qreal x = btnRect.x();
    qreal y = btnRect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    y += (btnRect.size().height() - h) / 2.0;
    x += (btnRect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
    return btnRect;
}

QRect TaskDelegate::iconRect(const QRect &itemRect) const
{
    QRect iconRect = itemRect;
    iconRect.setSize({ kTaskIconSize, kTaskIconSize });

    iconRect.moveLeft(iconRect.left() + kItemMargin);
    iconRect.moveTop(iconRect.top() + ((itemRect.bottom() - iconRect.bottom()) / 2));

    return iconRect;
}

QRect TaskDelegate::fixButtonRect(const QRect &itemRect) const
{
    QRect btnRect = itemRect;

    btnRect.setSize({ 20, 20 });
    btnRect.moveRight(itemRect.right() - kItemMargin);
    btnRect.moveTop(btnRect.top() + ((itemRect.bottom() - btnRect.bottom()) / 2));

    return btnRect;
}
