// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collapsewidget.h"
#include "detailsbutton.h"

#include <QLabel>
#include <QToolBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QtAlgorithms>

class CollapseWidgetPrivate
{
    friend class CollapseWidget;
    QString title;
    DetailsButton *detailsButton = nullptr;
    int defaultWidgetHeight = 500;
    int minHeight = 0;
    int maxHeight = 0;
    int widgetSpace = 8;
    DWidget *widget = nullptr;
    QRect titleLabelRect;
    QRect detailsButtonRect;
    QRect widgetRect;
    QVariantAnimation animation = nullptr;
};

CollapseWidget::CollapseWidget(QWidget *parent)
    : DWidget (parent)
    , d (new CollapseWidgetPrivate)
{
    setObjectName("CollapseWidget");
    setBackgroundRole(QPalette::Light);
    d->detailsButton = new DetailsButton(this);
    d->minHeight = d->detailsButton->height();
    d->maxHeight = d->detailsButton->height() + d->widgetSpace * 2;
    setMinimumWidth(260);

    d->animation.setDuration(200);
    setAutoFillBackground(true);

    QObject::connect(d->detailsButton, &DetailsButton::clicked,
                     this, &CollapseWidget::doChecked);

    QObject::connect(&d->animation, &QVariantAnimation::valueChanged,
                     this, &CollapseWidget::resetHeight, Qt::UniqueConnection);
}

CollapseWidget::CollapseWidget(const QString &title, QWidget *widget, QWidget *parent)
    :CollapseWidget(parent)
{
    setTitle(title);
    setWidget(widget);
}

CollapseWidget::~CollapseWidget()
{
    if (d) {
        if (d->detailsButton) {
            delete d->detailsButton;
        }
        if (d->widget) {
            delete d->widget;
        }
        delete d;
    }
}

DWidget *CollapseWidget::takeWidget()
{
    DWidget *result = nullptr;
    if (d->widget &&  this == d->widget->parent()) {
        d->widget->setParent(nullptr);
    }
    return result;
}

void CollapseWidget::setWidget(DWidget *widget)
{
    if (d->widget) {
        delete d->widget;
        d->widget = nullptr;
    }

    if (widget){
        d->widget = widget;
        d->widget->setParent(this);
        if (d->maxHeight == d->minHeight + d->widgetSpace * 2) {
            d->maxHeight += d->defaultWidgetHeight;
        }
        d->maxHeight = qMax(d->maxHeight, widget->height() + d->widgetSpace * 2);
        d->widget->resize(width(), d->maxHeight);
        d->widget->show();
    }
}

DWidget *CollapseWidget::widget()
{
    return d->widget;
}

void CollapseWidget::setTitle(const QString &title)
{
    d->title = title;
}

QString CollapseWidget::title()
{
    return d->title;
}

void CollapseWidget::setCheckable(bool checkable)
{
    if (d->detailsButton)
        d->detailsButton->setCheckable(checkable);
}

bool CollapseWidget::isCheckable()
{
    if (d->detailsButton)
        return d->detailsButton->isCheckable();
    return false;
}

bool CollapseWidget::isChecked()
{
    if (d->detailsButton)
        return d->detailsButton->isChecked();
    return false;
}

void CollapseWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);

    if (d->detailsButton) {
        d->detailsButtonRect = { QPoint(event->size().width() - d->detailsButton->width(), 0),
                                 d->detailsButton->size() };
        d->detailsButton->setGeometry(d->detailsButtonRect);
    }

    if (d->widget) {
        if (d->widget->width() != width() - d->widgetSpace * 2) {
            d->widget->resize(width() - d->widgetSpace * 2, d->widget->height());
        }
        d->maxHeight = qMax(d->widget->height() + d->widgetSpace * 2, d->maxHeight);
        QPoint widgetOffset = {d->widgetSpace, d->widgetSpace + d->minHeight};
        QSize widgetResize = event->size() - QSize{d->widgetSpace * 2, d->widgetSpace * 2 + d->minHeight};
        d->widget->setGeometry(QRect{widgetOffset, widgetResize});
    }
}

void CollapseWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
    QPainter painter(this);
    QRect rect = CollapseWidget::rect();
    painter.drawLine(rect.topLeft() + QPoint{0, d->minHeight -1},
                     rect.topRight() + QPoint{0, d->minHeight -1});
    painter.drawLine(rect.topLeft(), rect.topRight());
    painter.drawLine(rect.topLeft(), rect.bottomLeft());
    painter.drawLine(rect.topRight(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.bottomRight());

    QFont font(d->title);
    QFontMetrics fontMetrics(this->font());
    QRect textBoundingRect = fontMetrics.boundingRect(d->title);
    QSize fontSize = textBoundingRect.size();
    int fontPosX = textBoundingRect.x();
    int fontPosY = textBoundingRect.y();
    int textPosX = 4 + ( fontPosX > 0 ? fontPosX : - fontPosX);;
    int textPosY = (d->detailsButton->height() - fontSize.height()) / 2
            + ( fontPosY > 0 ? fontPosY: - fontPosY);
    painter.drawText(QPoint{textPosX, textPosY}, d->title);
}

void CollapseWidget::setChecked(bool checked)
{
    if (d->detailsButton) {
        d->detailsButton->setChecked(checked);
        this->doChecked(checked);
    }
}

void CollapseWidget::doChecked(bool checked)
{
    //默认展开状态, 点击时首先隐藏
    if (d->widget) {
        if (checked){
            d->widget->hide();
            d->animation.setStartValue(d->maxHeight);
            d->animation.setEndValue(d->minHeight);
        } else{
            d->widget->show();
            d->animation.setStartValue(d->minHeight);
            d->animation.setEndValue(qMax(d->maxHeight, d->minHeight));
        }
        d->animation.start();
    }
}

void CollapseWidget::resetHeight(const QVariant &value)
{
    setFixedHeight(value.toInt());
}
