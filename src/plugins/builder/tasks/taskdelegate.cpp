// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskdelegate.h"

#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#include <DPaletteHelper>
#endif

#include <QTextLayout>
#include <QPainter>
#include <QDir>
#include <QPainterPath>
#include <QPalette>

DWIDGET_USE_NAMESPACE

inline constexpr int kRectRadius = { 8 };

TaskDelegate::TaskDelegate(QAbstractItemView *parent) :
    DStyledItemDelegate(parent)
{ }

TaskDelegate::~TaskDelegate() = default;

QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    auto view = qobject_cast<const QAbstractItemView *>(opt.widget);
    const bool selected = (view->selectionModel()->currentIndex() == index);
    QSize s;
    s.setWidth(option.rect.width());

    if (!selected && option.font == cachedFont && cachedHeight > 0) {
        s.setHeight(cachedHeight);
        return s;
    }

    QFontMetrics fm(option.font);
    int fontHeight = fm.height();
    int fontLeading = fm.leading();

    auto model = static_cast<TaskFilterProxyModel *>(view->model());
    Positions positions(option, model);

    if (selected) {
        QString description = index.data(TaskModel::Description).toString();
        // Layout the description
        int leading = fontLeading;
        int height = 0;

        description.replace(QLatin1Char('\n'), QChar::LineSeparator);
        QTextLayout tl(description);
        tl.beginLayout();
        while (true) {
            QTextLine line = tl.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(positions.textAreaWidth());
            height += leading;
            line.setPosition(QPoint(0, height));
            height += static_cast<int>(line.height());
        }
        tl.endLayout();

        s.setHeight(height + leading + fontHeight + 3);
    } else {
        s.setHeight(fontHeight + 3);
    }
    if (s.height() < positions.minimumHeight())
        s.setHeight(positions.minimumHeight());

    if (!selected) {
        cachedHeight = s.height();
        cachedFont = option.font;
    }

    return s;
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
    paintItemBackground(painter, opt, index);
    painter->setOpacity(1);

    auto view = qobject_cast<const QAbstractItemView *>(opt.widget);
    bool selected = view->selectionModel()->currentIndex() == index;
    auto model = static_cast<TaskFilterProxyModel *>(view->model());
    Positions positions(opt, model);

    // Paint TaskIconArea:
    QPoint iconPos;

    iconPos = QPoint(positions.left() + 10,
                     positions.getTop() + (positions.getBottom() - positions.getTop() - positions.taskIconHeight()) / 2 );

    QIcon icon = index.data(TaskModel::Icon).value<QIcon>();
    painter->drawPixmap(iconPos, icon.pixmap(positions.taskIconWidth(), positions.taskIconHeight()));

    QRect textRect = option.rect.adjusted(30, 0, 6, 0);
    paintItemColumn(painter, opt, index, textRect);
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
                                          const QModelIndex &index, const QRectF &textRect) const
{
    painter->save();

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;

    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType){
        painter->setPen(QColor(255, 255, 255, 180));
    } else {
        painter->setPen(QColor(0, 0, 0, 180));
    }

    QString description = index.data(TaskModel::Description).toString();
    if (!isSelected)
        painter->drawText(textRect, Qt::AlignLeft, description);
    else {
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

    const QString directory = QDir::toNativeSeparators(index.data(TaskModel::File).toString());

    if (isSelected) {
        painter->setPen(QColor(Qt::white));
        if (index.data(TaskModel::FileNotFound).toBool() && !directory.isEmpty()) {
            QString fileNotFound = tr("File not found: %1").arg(directory);
            painter->setPen(Qt::red);
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignBottom, fileNotFound);
        } else {
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignBottom, directory);
        }
    }

    painter->restore();
}
