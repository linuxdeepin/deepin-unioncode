#include "outputwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QTextBrowser>

OutputWidget::OutputWidget(QWidget *parent) : QWidget(parent)
{
    QGridLayout *gridLayout = new QGridLayout(this);
    QTextBrowser *outputBrowser = new QTextBrowser(this);
    gridLayout->addWidget(outputBrowser);
    setLayout(gridLayout);
}
