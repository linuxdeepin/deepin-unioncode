#ifndef FILESOURCEDELEGATE_H
#define FILESOURCEDELEGATE_H

#include <QStyledItemDelegate>

class FileSourceDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FileSourceDelegate(QObject *parent = nullptr);
};

#endif // FILESOURCEDELEGATE_H
