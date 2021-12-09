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
    void createOptions(const QString &path);
};

#endif // TREEMENU_H
