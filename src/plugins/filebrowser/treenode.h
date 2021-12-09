#ifndef TREENODE_H
#define TREENODE_H

#include <QSharedData>

class Node
{
public:
    enum Type{Unknown, Files, Folders, File, Folder};

    explicit Node(Node* parent = nullptr,
                  const QString &text = "",
                  const QString &toolTip = "",
                  Type type = Unknown);

    virtual ~Node();
    Node *parent;
    QString text;
    QString toolTip;
    Type type;
    QList<Node*> children;
    bool isLoaded;
};
#endif // TREENODE_H
