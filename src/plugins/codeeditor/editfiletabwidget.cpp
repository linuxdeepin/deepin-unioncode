#include "editfiletabwidget.h"

#include <QDebug>

EditFileTabWidget::EditFileTabWidget(QWidget *parent)
    : QTabBar (parent)
{
    setExpanding(false);
}
