#include "configurewidget.h"
#include "environmentwidget.h"
#include "common/widget/collapsewidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

const QString runtimeEnvTitle {"Runtime Environment"};
const QString buildEnvTitle {"build Environment"};

class ConfigureWidgetPrivate
{
    friend class ConfigureWidget;
    QWidget *centrelWidget = nullptr;
    QVBoxLayout *layout = nullptr;
};

ConfigureWidget::ConfigureWidget(QWidget *parent)
    : QScrollArea(parent)
    , d(new ConfigureWidgetPrivate())
{
    setWidgetResizable(true);
    d->centrelWidget = new QWidget();
    d->layout = new QVBoxLayout(d->centrelWidget);
    d->centrelWidget->setLayout(d->layout);
    d->layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    setWidget(d->centrelWidget);
}

ConfigureWidget::~ConfigureWidget()
{
    if (d)
        delete d;
}

void ConfigureWidget::addCollapseWidget(CollapseWidget *widget)
{
    if (!widget)
        return;
    qInfo() << widget->isChecked();
    if (!widget->isChecked()) {
        widget->setChecked(true);
    }
    widget->setFixedHeight(600);
    d->layout->insertWidget(d->layout->count() - 1, widget);
    qInfo() << widget->size();
}

void ConfigureWidget::resizeEvent(QResizeEvent *event)
{
    d->centrelWidget->resize(d->centrelWidget->width(), event->size().width());
    QScrollArea::resizeEvent(event);
}
