#ifndef CMAKEITEMKEEPER_H
#define CMAKEITEMKEEPER_H

#include <QObject>
#include <QStandardItem>

class GradleItemKeeperPrivate;
class GradleItemKeeper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GradleItemKeeper)
    GradleItemKeeper();
    GradleItemKeeperPrivate *const d;

public:
    static GradleItemKeeper *instance();
    virtual ~GradleItemKeeper();
};

#endif // CMAKEITEMKEEPER_H
