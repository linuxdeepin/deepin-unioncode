#include "gradleitemkeeper.h"
#include "common/common.h"

namespace  {
static GradleItemKeeper *ins {nullptr};
};

/*!
 * \brief The MavenItemKeeperPrivate class
 */
class GradleItemKeeperPrivate
{
    friend class MavenItemKeeper;
    QHash<QString, QStandardItem*> child();
};

/*!
 * \brief MavenItemKeeper::MavenItemKeeper
 */
GradleItemKeeper::GradleItemKeeper()
    : d (new GradleItemKeeperPrivate)
{
}

GradleItemKeeper *GradleItemKeeper::instance() {
    if (!::ins)
        ::ins = new GradleItemKeeper();
    return ins;
}

GradleItemKeeper::~GradleItemKeeper()
{
    if (d)
        delete d;
}
