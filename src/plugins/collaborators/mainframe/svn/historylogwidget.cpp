#include "historylogwidget.h"
#include "historyview.h"
#include "filemodifyview.h"

#include <QTextBrowser>

HistoryLogWidget::HistoryLogWidget(QWidget *parent)
    : QSplitter (parent)
    , descBrowse(new QTextBrowser)
    , hisView(new HistoryView)
    , changedView(new FileModifyView)
{
    descBrowse->setMinimumHeight(60);
    descBrowse->setPlaceholderText(QTextBrowser::tr("Description from revision log"));
    hisView->setMinimumHeight(300);
    changedView->setMinimumHeight(300);
    setOrientation(Qt::Vertical);
    addWidget(descBrowse);
    setCollapsible(0, false);
    addWidget(changedView);
    addWidget(hisView);
    setCollapsible(1, false);
    setHandleWidth(2);
    QObject::connect(hisView, &HistoryView::clicked, [=](const QModelIndex &index){
        descBrowse->setText(hisView->description(index.row()));
        changedView->setFiles(hisView->revisionFiles(index.row()));
    });
}

QTextBrowser *HistoryLogWidget::descriptionBrowse()
{
    return descBrowse;
}

HistoryView *HistoryLogWidget::historyView()
{
    return hisView;
}

FileModifyView *HistoryLogWidget::fileChangedView()
{
    return changedView;
}
