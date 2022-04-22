#include "reposwidget.h"
#include "amendswidget.h"
#include "commithistorywidget.h"

#include <QLabel>
#include <QSet>
#include <QSplitter>

ReposWidget::ReposWidget(QWidget *parent)
    : QSplitter(parent)
    , amendsWidget(new AmendsWidget)
    , commitHisWidget(new CommitHistoryWidget)
{
    setOrientation(Qt::Horizontal);
    //    auto label1 = new QLabel("Test1");
    //    auto label2 = new QLabel("Test2");
    //    label1->setStyleSheet("background-color:red");
    //    label2->setStyleSheet("background-color:green");
    addWidget(amendsWidget);
    addWidget(commitHisWidget);
    //    addWidget(label1);
    //    addWidget(label2);
    amendsWidget->reflashAmends();
    setCollapsible(0, false);
}
