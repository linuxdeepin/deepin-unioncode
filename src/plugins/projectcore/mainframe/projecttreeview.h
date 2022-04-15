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
Q_SIGNALS:
    void indexMenuRequest(const QModelIndex &index, QContextMenuEvent *event);
    void itemMenuRequest(const QStandardItem *item, QContextMenuEvent *event);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // PROJECTTREEVIEW_H
