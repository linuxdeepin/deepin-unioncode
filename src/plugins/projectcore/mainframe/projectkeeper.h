#ifndef PROJECTKEEPER_H
#define PROJECTKEEPER_H

#include <QObject>

class QAction;
class ProjectTreeView;
class QStandardItem;
class ProjectKeeper final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectKeeper)
public:
    ProjectKeeper();
    static ProjectKeeper *instance();
    ProjectTreeView *treeView();
};

#endif // PROJECTKEEPER_H
