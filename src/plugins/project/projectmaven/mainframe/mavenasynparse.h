#ifndef CMAKEASYNPARSE_H
#define CMAKEASYNPARSE_H

#include "services/project/projectinfo.h"

#include "common/common.h"

#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;
class MavenAsynParsePrivate;
class MavenAsynParse : public Inotify
{
    Q_OBJECT
    friend class MavenGenerator;
    MavenAsynParsePrivate *const d;
public:
    MavenAsynParse();
    virtual ~MavenAsynParse();

signals:
    void parsedActions(const dpfservice::ParseInfo<dpfservice::ProjectActionInfos> &info);
    void parsedError(const dpfservice::ParseInfo<QString> &info);
    void itemsModified(const dpfservice::ParseInfo<QList<QStandardItem*>> &info);

public slots:
    void loadPoms(const dpfservice::ProjectInfo &info);
    void parseProject(const dpfservice::ProjectInfo &info);
    void parseActions(const dpfservice::ProjectInfo &info);

private slots:
    void doDirWatchModify(const QString &path);
    void doWatchCreatedSub(const QString &path);
    void doWatchDeletedSub(const QString &path);

private:
    bool isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role = Qt::DisplayRole) const;
    void createRows(const QString &path);
    void removeRows();
    void removeSelfSubWatch(QStandardItem *item);
    QList<QStandardItem*> rows(const QStandardItem *item) const;
    int findRowWithDisplay(QList<QStandardItem*> rowList, const QString &fileName);
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    int separatorSize() const;
    bool itemIsDir(const QStandardItem *item) const;
    bool itemIsFile(const QStandardItem *item) const;
    QStringList pathChildFileNames(const QString &path) const;
    QStringList displayNames(const QList<QStandardItem*> items) const;
    QStringList createdFileNames(const QString &path) const;
    QStringList deletedFileNames(const QString &path) const;
};

#endif // CMAKEASYNPARSE_H
