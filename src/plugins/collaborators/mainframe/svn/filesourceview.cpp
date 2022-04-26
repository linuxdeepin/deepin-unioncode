#include "filesourceview.h"

#include <QGridLayout>
#include <QFileSystemModel>

class FileSourceViewPrivate
{
    friend class FileSourceView;
    QFileSystemModel *model{nullptr};
};

FileSourceView::FileSourceView(QWidget *parent)
    : QTreeView(parent)
    , d (new FileSourceViewPrivate)
{
    d->model = new QFileSystemModel;
    d->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
    setModel(d->model);
}

void FileSourceView::setRootPath(const QString &filePath)
{
    d->model->setRootPath(filePath);
    setRootIndex(d->model->index(filePath));
}
