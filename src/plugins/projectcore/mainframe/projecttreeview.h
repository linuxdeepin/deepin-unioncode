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
    void appendRootItem(QStandardItem *root);
    void removeRootItem(const QStandardItem *root);
    void expandedProjectDepth(const QStandardItem *root, int depth);
    void expandedProjectAll(const QStandardItem *root);
Q_SIGNALS:
    void indexMenuRequest(const QModelIndex &index, QContextMenuEvent *event);
    void itemMenuRequest(const QStandardItem *item, QContextMenuEvent *event);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void doItemMenuRequest(const QStandardItem *item, QContextMenuEvent *event);
    void doDoubleClieked(const QModelIndex &index);
};

#endif // PROJECTTREEVIEW_H
