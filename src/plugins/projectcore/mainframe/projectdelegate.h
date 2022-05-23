#ifndef PROJECTDELEGATE_H
#define PROJECTDELEGATE_H

#include <QStyledItemDelegate>

class ProjectDelegatePrivate;
class ProjectDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    ProjectDelegatePrivate *const d;
public:
    explicit ProjectDelegate(QObject *parent = nullptr);
    void setActiveProject(const QModelIndex &root);

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // PROJECTDELEGATE_H
