#include "environmentwidget.h"
#include <QVBoxLayout>
#include <QCheckBox>

const QString ENABLE_ALL_ENV = EnvironmentWidget::tr("Enable All Environment");

class EnvironmentWidgetPrivate
{
    friend class EnvironmentWidget;
    QVBoxLayout *vLayout = nullptr;
    QTableView *tableView = nullptr;
    QCheckBox *checkBox = nullptr;
};

EnvironmentWidget::EnvironmentWidget(QWidget *parent)
    : QWidget (parent)
    , d(new EnvironmentWidgetPrivate)
{
    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->tableView)
        d->tableView = new QTableView();

    if (!d->checkBox)
        d->checkBox = new QCheckBox();
    d->checkBox->setText(ENABLE_ALL_ENV);
    d->vLayout->setSpacing(0);
    d->vLayout->setMargin(0);
    d->vLayout->addWidget(d->checkBox);
    d->vLayout->addWidget(d->tableView);
}

EnvironmentWidget::~EnvironmentWidget()
{
    if(d)
        delete d;
}
