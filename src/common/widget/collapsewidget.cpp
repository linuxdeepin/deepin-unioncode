#include "collapsewidget.h"
#include "detailsbutton.h"
#include "omittablelabel.h"

#include <QLabel>
#include <QToolBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QDebug>
#include <QEvent>
#include <QtAlgorithms>

class CollapseWidgetPrivate
{
    friend class CollapseWidget;
    OmittableLabel *titleLabel = nullptr;
    DetailsButton *detailsButton = nullptr;
    int defaultWidgetHeight = 500;
    int minHeight = 0;
    int maxHeight = 0;
    QRect titleLabelRect;
    QRect detailsButtonRect;
    QRect widgetRect;
    QWidget *widget = nullptr;
    QVariantAnimation animation = nullptr;
};

CollapseWidget::CollapseWidget(QWidget *parent)
    : QWidget (parent)
    , d (new CollapseWidgetPrivate)
{
    setBackgroundRole(QPalette::Light);
    d->titleLabel = new OmittableLabel(this);
    d->detailsButton = new DetailsButton(this);
    d->widget = new QWidget(this);
    d->minHeight = qMax(d->titleLabel->height(), d->detailsButton->height());
    d->maxHeight = qMax(d->widget->height(), d->defaultWidgetHeight + d->minHeight);
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
    if (d)
        delete d;
}

QWidget *CollapseWidget::takeWidget()
{
    QWidget *result = nullptr;
    if (d->widget &&  this == d->widget->parent()) {
        d->widget->setParent(nullptr);
    }
    return result;
}

void CollapseWidget::setWidget(QWidget *widget)
{
    if (d->widget) {
        d->widget->setParent(this);
        delete d->widget;
        d->widget = nullptr;

        if (widget){
            d->widget = widget;
            d->widget->setParent(this);
            d->maxHeight = qMax(widget->height(), d->maxHeight);
            d->widget->show();
        }
    }
}

QWidget *CollapseWidget::widget()
{
    return d->widget;
}

void CollapseWidget::setTitle(const QString &title)
{
    if (d->titleLabel)
        d->titleLabel->setText(title);
}

QString CollapseWidget::title()
{
    if (d->titleLabel)
        return d->titleLabel->text();
    return "";
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
    d->detailsButtonRect = QRect(QPoint(event->size().width() - d->detailsButton->width(), 0),
                                 d->detailsButton->size()); //button rect
    d->titleLabelRect = QRect(QPoint(0, 0), d->titleLabel->size());
    d->widgetRect = QRect(QPoint(0, d->minHeight), d->widget->size());
    d->detailsButton->resize(d->detailsButton->width(), d->minHeight);
    d->titleLabel->resize(event->size().width() - d->detailsButton->width(), d->minHeight);

    if (d->titleLabel)
        d->titleLabel->setGeometry(d->titleLabelRect);

    if (d->detailsButton)
        d->detailsButton->setGeometry(d->detailsButtonRect);

    if(d->widget)
        d->widget->setGeometry(0, d->minHeight, width(), d->widget->height() + d->minHeight);

    QWidget::resizeEvent(event);
}

void CollapseWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
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
            d->animation.setStartValue(qMax(d->maxHeight, d->minHeight));
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
