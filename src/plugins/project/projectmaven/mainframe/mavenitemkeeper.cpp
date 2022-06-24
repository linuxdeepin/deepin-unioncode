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
    QHash<QStandardItem*, QPair<QString, QStringList>> itemCmakeFileNodes;
};

/*!
 * \brief MavenItemKeeper::MavenItemKeeper
 */
MavenItemKeeper::MavenItemKeeper()
    : d (new MavenItemKeeperPrivate)
{
    // 全量变动监听，避免因为业务逻辑导致无法监听某个文件
    //    QObject::connect(Inotify::globalInstance(), &Inotify::modified,
    //                     this, &MavenItemKeeper::notifyFromWatcher,
    //                     Qt::ConnectionType::BlockingQueuedConnection);

    //    QObject::connect(Inotify::globalInstance(), &Inotify::ignoreModified,
    //                     this, &MavenItemKeeper::notifyFromWatcher,
    //                     Qt::ConnectionType::BlockingQueuedConnection);
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

void MavenItemKeeper::addCmakeRootFile(QStandardItem *root, const QString rootPath)
{
    d->itemCmakeFileNodes[root].first = rootPath;
    Inotify::globalInstance()->addPath(rootPath);
    Inotify::globalInstance()->removeIgnorePath(rootPath);
}

void MavenItemKeeper::addCmakeSubFiles(QStandardItem *root, const QStringList subPaths)
{
    d->itemCmakeFileNodes[root].second = subPaths;
    for (auto &val : subPaths) {
        Inotify::globalInstance()->addPath(val);
        Inotify::globalInstance()->removeIgnorePath(val);
    }
}

void MavenItemKeeper::delCmakeFileNode(QStandardItem *rootItem)
{
    Inotify::globalInstance()->removePath(d->itemCmakeFileNodes.value(rootItem).first);
    auto subFiles = d->itemCmakeFileNodes.value(rootItem).second;
    for (auto &val : subFiles) {
        Inotify::globalInstance()->removePath(val);
    }

    d->itemCmakeFileNodes.remove(rootItem);
}

void MavenItemKeeper::notifyFromWatcher(const QString &filePath)
{
    for (auto val: d->itemCmakeFileNodes) {
        if (val.second.contains(filePath)
                || val.first == filePath) {
            emit cmakeFileNodeNotify(d->itemCmakeFileNodes.key(val), val);
            break; //后续会出发 delete操作 再执行foreach将会崩溃
        }
    }
}
