#include "cmakeitemkeeper.h"
#include "common/common.h"

namespace  {
static CmakeItemKeeper *ins {nullptr};
};

/*!
 * \brief The CmakeItemKeeperPrivate class
 */
class CmakeItemKeeperPrivate
{
    friend class CmakeItemKeeper;
    QHash<QStandardItem*, QPair<QString, QStringList>> itemCmakeFileNodes;
};

/*!
 * \brief CmakeItemKeeper::CmakeItemKeeper
 */
CmakeItemKeeper::CmakeItemKeeper()
    : d (new CmakeItemKeeperPrivate)
{
    // 全量变动监听，避免因为业务逻辑导致无法监听某个文件
    QObject::connect(Inotify::globalInstance(), &Inotify::modified,
                     this, &CmakeItemKeeper::notifyFromWatcher);

    QObject::connect(Inotify::globalInstance(), &Inotify::ignoreModified,
                     this, &CmakeItemKeeper::notifyFromWatcher);
}

CmakeItemKeeper *CmakeItemKeeper::instance() {
    if (!::ins)
        ::ins = new CmakeItemKeeper();
    return ins;
}

CmakeItemKeeper::~CmakeItemKeeper()
{
    if (d)
        delete d;
}

void CmakeItemKeeper::addCmakeRootFile(QStandardItem *root, const QString rootPath)
{
    d->itemCmakeFileNodes[root].first = rootPath;
    Inotify::globalInstance()->addPath(rootPath);
    Inotify::globalInstance()->removeIgnorePath(rootPath);
}

void CmakeItemKeeper::addCmakeSubFiles(QStandardItem *root, const QStringList subPaths)
{
    d->itemCmakeFileNodes[root].second = subPaths;
    for (auto &val : subPaths) {
        Inotify::globalInstance()->addPath(val);
        Inotify::globalInstance()->removeIgnorePath(val);
    }
}

void CmakeItemKeeper::delCmakeFileNode(QStandardItem *rootItem)
{
    Inotify::globalInstance()->removePath(d->itemCmakeFileNodes.value(rootItem).first);
    auto subFiles = d->itemCmakeFileNodes.value(rootItem).second;
    for (auto &val : subFiles) {
        Inotify::globalInstance()->removePath(val);
    }

    d->itemCmakeFileNodes.remove(rootItem);
}

void CmakeItemKeeper::notifyFromWatcher(const QString &filePath)
{
    for (auto val: d->itemCmakeFileNodes) {
        if (val.second.contains(filePath)
                || val.first == filePath) {
            emit cmakeFileNodeNotify(d->itemCmakeFileNodes.key(val), val);
            break; //后续会出发 delete操作 再执行foreach将会崩溃
        }
    }
}
