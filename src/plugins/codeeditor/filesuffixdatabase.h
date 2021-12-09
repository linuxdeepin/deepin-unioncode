#ifndef FILESUFFIXDATABASE_H
#define FILESUFFIXDATABASE_H

#include <QObject>

class FileSuffixDatabase : public QObject
{
    Q_OBJECT
public:
    explicit FileSuffixDatabase(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FILESUFFIXDATABASE_H