#include "commithistorywidget.h"
#include "commithistoryview.h"
#include "commithistorymodel.h"

using namespace collaborators;

CommitHistoryWidget::CommitHistoryWidget(QWidget *parent)
    : QWidget(parent)
    , view(new CommitHistoryView)
{

}
