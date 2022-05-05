#include "filesourceview.h"
#include "filesourcedelegate.h"

#include <QFileSystemModel>
#include <QGridLayout>

class FileSourceViewPrivate
{
    friend class FileSourceView;
    QFileSystemModel *model{nullptr};
    FileSourceDelegate *delegate{nullptr};
};

FileSourceView::FileSourceView(QWidget *parent)
    : QTreeView(parent)
    , d (new FileSourceViewPrivate)
{
    d->model = new QFileSystemModel;
    d->delegate = new FileSourceDelegate;
    d->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
    setModel(d->model);
    setItemDelegate(d->delegate);
}

void FileSourceView::setRootPath(const QString &filePath)
{
    d->model->setRootPath(filePath);
    setRootIndex(d->model->index(filePath));
}
