#include "projectselectionmodel.h"

ProjectSelectionModel::ProjectSelectionModel(QAbstractItemModel *model)
    : QItemSelectionModel (model)
{

}

ProjectSelectionModel::ProjectSelectionModel(QAbstractItemModel *model, QObject *parent)
    : QItemSelectionModel (model, parent)
{

}
