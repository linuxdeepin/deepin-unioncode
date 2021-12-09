#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

class Node;
class TreeViewPrivate;
class TreeView : public QTreeView
{
    Q_OBJECT
    TreeViewPrivate * const d;
public:
    explicit TreeView(QWidget *parent = nullptr);
    virtual ~TreeView() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void clickedFile(const QString &path);

private slots:
    void selectAppended(Node *node);
    void selectRenamed(Node *node);
    void closeTheFile(Node *node);
    void closeTheFolder(Node *node);
    void closeAll(Node *node);
    void rename(Node *node);
    void doubleClicked(const QModelIndex &index);
};

#endif // TREEVIEW_H
