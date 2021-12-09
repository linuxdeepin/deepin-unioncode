#ifndef TREEPROXYWATCHER_H
#define TREEPROXYWATCHER_H

#include <QObject>

class Node;
class TreeNodeWatcherPrivate;
class TreeNodeWatcher : public QObject
{
    Q_OBJECT
    TreeNodeWatcherPrivate *const d;

public:
    explicit TreeNodeWatcher(QObject *parent = nullptr);
    virtual ~TreeNodeWatcher();
    bool hasNode(Node *node);
    void append(Node *node);
    void remove(Node *node);

signals:
    void dataChanged(Node *node);
};

#endif // TREEPROXYWATCHER_H
