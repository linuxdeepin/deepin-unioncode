#include "virtualtabwidget.h"

VirtualTabWidget::VirtualTabWidget() :
    tabWidget(new QTabWidget())
{
    tabWidget->setTabPosition(QTabWidget::South);
    tabWidget->setTabsClosable(true);
}

VirtualTabWidget::~VirtualTabWidget()
{
    delete tabWidget;
}

QTabWidget *VirtualTabWidget::getTabWidget()
{
    return  tabWidget;
}
