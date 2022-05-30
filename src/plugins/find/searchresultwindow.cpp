#include "searchresultwindow.h"

#include <QVBoxLayout>
#include <QPushButton>

SearchResultWindow::SearchResultWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SearchResultWindow::setupUi()
{
    QPushButton *researchBtn = new QPushButton("Re-Search");
    researchBtn->setFixedHeight(30);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(researchBtn, 0, Qt::AlignRight);

    SearchResultTreeView *treeView = new SearchResultTreeView();
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout);
    vLayout->addWidget(treeView);

    connect(researchBtn, &QPushButton::clicked, this, &SearchResultWindow::research);

    setLayout(vLayout);
}

void SearchResultWindow::research()
{
    emit back();
}
