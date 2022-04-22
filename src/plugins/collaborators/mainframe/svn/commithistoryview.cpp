#include "commithistoryview.h"
#include "commithistorymodel.h"

namespace collaborators {

CommitHistoryView::CommitHistoryView(QWidget *parent)
    : QTreeView(parent)
    , model(new CommitHistoryModel)
{
    setModel(model);
}

} // namespace collaborators
