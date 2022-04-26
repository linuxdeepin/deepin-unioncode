#include "historydisplaywidget.h"
#include "historylogwidget.h"
#include "historydiffwidget.h"

#include <QTextBrowser>

HistoryDisplayWidget::HistoryDisplayWidget(QWidget *parent)
    : QSplitter (parent)
    , hisLogWidget (new HistoryLogWidget)
    , hisDiffWidget (new HistoryDiffWidget)
{
    hisLogWidget->setMinimumWidth(300);
    hisDiffWidget->setMinimumWidth(300);
    setOrientation(Qt::Horizontal);
    setHandleWidth(2);
    addWidget(hisLogWidget);
    setCollapsible(0, false);
    addWidget(hisDiffWidget);
    setCollapsible(1, false);
}

HistoryLogWidget *HistoryDisplayWidget::logWidget()
{
    return hisLogWidget;
}

HistoryDiffWidget *HistoryDisplayWidget::diffWidget()
{
    return hisDiffWidget;
}
