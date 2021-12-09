#ifndef TREEMENU_H
#define TREEMENU_H

#include <QMenu>

class TreeMenuPrivate;
class TreeMenu : public QMenu
{
    Q_OBJECT
    TreeMenuPrivate *const d;
public:
    explicit TreeMenu(QWidget *parent = nullptr);
    virtual ~TreeMenu();
    void createNewFileAction(const QString &path);
    void createNewFolderAction(const QString &path);
    void createMoveToTrash(const QString &path);
    void createDeleteAction(const QString &path);
    void createBuildAction(const QString &path);
};

#endif // TREEMENU_H
