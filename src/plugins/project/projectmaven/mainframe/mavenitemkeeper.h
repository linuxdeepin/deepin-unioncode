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

signals:
    void cmakeFileNodeNotify(QStandardItem *rootItem, const QPair<QString, QStringList> &files);

public slots:
    void addCmakeRootFile(QStandardItem *root, const QString rootPath);
    void addCmakeSubFiles(QStandardItem *root, const QStringList subPaths);
    void delCmakeFileNode(QStandardItem *rootItem);

private:
    void notifyFromWatcher(const QString &filePath);
};

#endif // CMAKEITEMKEEPER_H
