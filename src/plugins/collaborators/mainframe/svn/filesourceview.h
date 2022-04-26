#ifndef FILESOURCEVIEW_H
#define FILESOURCEVIEW_H

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

signals:
    void menuRequest(const QString &file, const QPoint &global);

public slots:
};

#endif // FILESOURCEVIEW_H
