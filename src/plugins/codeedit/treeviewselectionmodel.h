#ifndef TREEVIEWSELECTIONMODEL_H
#define TREEVIEWSELECTIONMODEL_H


#include <QItemSelectionModel>

class TreeViewSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    explicit TreeViewSelectionModel(QAbstractItemModel *model = nullptr);

};

#endif // TREEVIEWSELECTIONMODEL_H
