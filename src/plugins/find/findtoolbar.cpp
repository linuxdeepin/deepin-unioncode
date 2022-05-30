#include "findtoolbar.h"

#include "currentdocumentfind.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#define LABEL_WIDTH           (80)
#define OPERATOR_WIDGET_WIDTH (400)

class FindToolBarPrivate
{
    CurrentDocumentFind *currentDocumentFind;

    friend class FindToolBar;
};

FindToolBar::FindToolBar(CurrentDocumentFind *currentDocumentFind)
{
    //d->currentDocumentFind = currentDocumentFind;
    setupUi();
}

FindToolBar::~FindToolBar()
{

}

void FindToolBar::setupUi()
{
    setMaximumHeight(85);
    setMinimumWidth(800);

    QVBoxLayout *vLayout = new QVBoxLayout();
    QHBoxLayout *findLayout = new QHBoxLayout();
    vLayout->addLayout(findLayout);
    QLabel *findLabel = new QLabel("Find:");
    findLabel->setFixedWidth(LABEL_WIDTH);
    findLabel->setAlignment(Qt::AlignRight);

    QLineEdit *findLineEdit = new QLineEdit();

    QWidget *findWidget = new QWidget();
    findWidget->setFixedWidth(OPERATOR_WIDGET_WIDTH);
    QHBoxLayout *findWidgetLayout = new QHBoxLayout();
    findWidgetLayout->setMargin(0);
    findWidget->setLayout(findWidgetLayout);

    QPushButton *findPreBtn = new QPushButton("Find Previous");
    QPushButton *findNextBtn = new QPushButton("Find Next");
    QPushButton *advancedBtn = new QPushButton("Advanced...");
    findWidgetLayout->addWidget(findPreBtn);
    findWidgetLayout->addWidget(findNextBtn);
    findWidgetLayout->addWidget(advancedBtn);

    findLayout->addWidget(findLabel);
    findLayout->addWidget(findLineEdit);
    findLayout->addWidget(findWidget);

    QHBoxLayout *repalceLayout = new QHBoxLayout();
    vLayout->addLayout(repalceLayout);
    QLabel *repalceLabel = new QLabel("Repalce:");
    repalceLabel->setFixedWidth(LABEL_WIDTH);
    repalceLabel->setAlignment(Qt::AlignRight);

    QLineEdit *replaceLineEdit = new QLineEdit();

    QWidget *replaceWidget = new QWidget();
    replaceWidget->setFixedWidth(OPERATOR_WIDGET_WIDTH);
    QHBoxLayout *replaceWidgetLayout = new QHBoxLayout();
    replaceWidgetLayout->setMargin(0);
    replaceWidget->setLayout(replaceWidgetLayout);

    QPushButton *replaceBtn = new QPushButton("Replace");
    QPushButton *replaceFindBtn = new QPushButton("Replace && Find");
    QPushButton *replaceAllBtn = new QPushButton("Repalce All");
    replaceWidgetLayout->addWidget(replaceBtn);
    replaceWidgetLayout->addWidget(replaceFindBtn);
    replaceWidgetLayout->addWidget(replaceAllBtn);

    repalceLayout->addWidget(repalceLabel);
    repalceLayout->addWidget(replaceLineEdit);
    repalceLayout->addWidget(replaceWidget);

    connect(findPreBtn, &QAbstractButton::clicked, this, &FindToolBar::findPrevious);
    connect(findNextBtn, &QAbstractButton::clicked, this, &FindToolBar::findNext);
    connect(advancedBtn, &QAbstractButton::clicked, this, &FindToolBar::advancedSearch);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolBar::replace);
    connect(replaceFindBtn, &QAbstractButton::clicked, this, &FindToolBar::replaceSearch);
    connect(replaceAllBtn, &QAbstractButton::clicked, this, &FindToolBar::replaceAll);


    setLayout(vLayout);
}

void FindToolBar::findPrevious()
{

}

void FindToolBar::findNext()
{

}

void FindToolBar::advancedSearch()
{
    emit advanced();
}

void FindToolBar::replace()
{

}

void FindToolBar::replaceSearch()
{

}

void FindToolBar::replaceAll()
{

}

