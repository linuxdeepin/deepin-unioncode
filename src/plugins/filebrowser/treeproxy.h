#ifndef TREEPROXY_H
#define TREEPROXY_H

#include "treenode.h"

#include <QObject>
#include <QSharedData>

class QMutex;
class TreeProxyPrivate;
class TreeProxy : public QObject
{
    Q_OBJECT
    friend class TreeSelectionModel;
    friend class TreeView;
    friend class TreeModel;
    TreeProxyPrivate *const d;

private:
    explicit TreeProxy(QObject *parent = nullptr);
    // forever not nullptr, cause in class stack
    Node* files() const;
    // forever not nullptr, cause in class stack
    Node* folders() const;
    Node* root(Node *node) const;
    Node* rootFromFolder(Node *node) const;
    Node* rootFromFiles(Node *node) const;
    void loadChildren(Node *node);
    QList<int> parentsRowFind(Node *node) const;
    int row(Node *node);
    void appendWatcherNode(Node *node);
    void removeWatcherNode(Node *node);

public:
    virtual ~TreeProxy();
    static TreeProxy &instance();
    bool hasChild(Node *node, const QString &text,
                  const QString &toolTip) const;
    void appendFile(const QString &path);
    void appendFolder(const QString &path);
    void createFile(const QString &path);
    void createFolder(const QString &path);
    void deleteFile(const QString &path);
    void deleteFolder(const QString &path);

signals:
    void createdFile(const QString &path);
    void createdFolder(const QString &path);
    void appendedFile(Node *node);
    void appendedFolder(Node *node);
    void dataChangedFromWatcher(Node *node);
};

#endif // TREEPROXY_H
