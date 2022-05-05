#ifndef FILESOURCEVIEW_H
#define FILESOURCEVIEW_H

#include "basetype.h"

#include <QTreeView>

class QVBoxLayout;
class FileSourceViewPrivate;
class FileSourceView : public QTreeView
{
    Q_OBJECT
    FileSourceViewPrivate *const d;
public:
    explicit FileSourceView(QWidget *parent = nullptr);
    void setRootPath(const QString &filePath);
};

#endif // FILESOURCEVIEW_H
