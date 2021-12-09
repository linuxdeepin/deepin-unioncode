#ifndef TREEVIEWDELEGATE_H
#define TREEVIEWDELEGATE_H

#include <QItemDelegate>

class TreeViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit TreeViewDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
};

#endif // TREEVIEWDELEGATE_H
