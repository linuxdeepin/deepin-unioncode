#ifndef FILESOURCEMODEL_H
#define FILESOURCEMODEL_H

#include <QFileSystemModel>

class FileSourceModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit FileSourceModel(QObject *parent = nullptr);
};

#endif // FILESOURCEMODEL_H
