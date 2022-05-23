#include "projectdelegate.h"

#include <QPainter>

class ProjectDelegatePrivate
{
    friend class ProjectDelegate;
    QModelIndex index;
};

ProjectDelegate::ProjectDelegate(QObject *parent)
  : QStyledItemDelegate (parent)
  , d (new ProjectDelegatePrivate)
{

}

void ProjectDelegate::setActiveProject(const QModelIndex &root)
{
    d->index = root;
}

void ProjectDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    QStyleOptionViewItem iOption = option;
    if (d->index.isValid() && d->index == index) {
        iOption.font.setBold(true);
    }
    QStyledItemDelegate::paint(painter, iOption, index);
}
