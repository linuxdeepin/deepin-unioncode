#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

class TreeViewPrivate;
class TreeView : public QTreeView
{
    Q_OBJECT
    TreeViewPrivate *const d;
public:
    explicit TreeView(QWidget *parent = nullptr);
    virtual ~TreeView();
    void setRootPath(const QString  &rootPath);
public slots:
    void selOpen();
    void selMoveToTrash();
    void selRemove();
    void selNewDocument();
    void selNewFolder();
    void recoverFromTrash();
signals:
    void rootPathChanged(const QString &folder);
protected:
    void contextMenuEvent(QContextMenuEvent *event);
    virtual QMenu *createContextMenu(const QModelIndexList &indexs);
    virtual QMenu *createEmptyMenu();
};

#endif // TREEVIEW_H
