#ifndef CMAKEITEMKEEPER_H
#define CMAKEITEMKEEPER_H

#include <QObject>
#include <QStandardItem>

class MavenItemKeeperPrivate;
class MavenItemKeeper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MavenItemKeeper)
    MavenItemKeeper();
    MavenItemKeeperPrivate *const d;

public:
    static MavenItemKeeper *instance();
    virtual ~MavenItemKeeper();
};

#endif // CMAKEITEMKEEPER_H
