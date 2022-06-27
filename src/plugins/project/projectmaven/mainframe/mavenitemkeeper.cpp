#include "mavenitemkeeper.h"
#include "common/common.h"

namespace  {
static MavenItemKeeper *ins {nullptr};
};

/*!
 * \brief The MavenItemKeeperPrivate class
 */
class MavenItemKeeperPrivate
{
    friend class MavenItemKeeper;
    QHash<QString, QStandardItem*> child();
};

/*!
 * \brief MavenItemKeeper::MavenItemKeeper
 */
MavenItemKeeper::MavenItemKeeper()
    : d (new MavenItemKeeperPrivate)
{
}

MavenItemKeeper *MavenItemKeeper::instance() {
    if (!::ins)
        ::ins = new MavenItemKeeper();
    return ins;
}

MavenItemKeeper::~MavenItemKeeper()
{
    if (d)
        delete d;
}
