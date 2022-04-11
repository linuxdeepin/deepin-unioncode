#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include <QTreeView>

class QStandardItem;
class ProjectTreeViewPrivate;
class ProjectTreeView : public QTreeView
{
    Q_OBJECT
    ProjectTreeViewPrivate *const d;
public:
    explicit ProjectTreeView(QWidget *parent = nullptr);
    virtual ~ProjectTreeView();
    void appendRootItem(QStandardItem *item);
    void expandedProjectDepth(QStandardItem *rootItem, int depth);
    void expandedProjectAll(QStandardItem *rootItem);
};

#endif // PROJECTTREEVIEW_H
