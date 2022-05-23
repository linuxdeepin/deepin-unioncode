#ifndef PROJECTSELECTIONMODEL_H
#define PROJECTSELECTIONMODEL_H

#include <QItemSelectionModel>

class ProjectSelectionModel : public QItemSelectionModel
{
public:
    explicit ProjectSelectionModel(QAbstractItemModel *model = nullptr);
    explicit ProjectSelectionModel(QAbstractItemModel *model, QObject *parent);
};

#endif // PROJECTSELECTIONMODEL_H
