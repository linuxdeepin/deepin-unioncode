#include "treemenu.h"
#include "treeproxy.h"
#include "sendevents.h"
#include "common/util/custompaths.h"
#include "common/util/processutil.h"

#include <QFileInfo>
#include <QAction>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QAbstractButton>
#include <QFileSystemWatcher>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QApplication>
#include <QStandardPaths>

const QString BUILD_SUPPORT_FILE_NAME {"BuilderSupport.json"};
const QString NEW_FILE {TreeMenu::tr("New File")};
const QString NEW_FOLDER {TreeMenu::tr("New Folder")};
const QString MOVE_TO_TARSH {TreeMenu::tr("Move To Trash")};
const QString DELETE {TreeMenu::tr("Delete")};
const QString BUILD {TreeMenu::tr("Build")};

class TreeMenuPrivate
{
    friend class TreeMenu;
    QJsonDocument globalJsonDocument;
    QJsonDocument cacheJsonDocument;
    void initBuildSupport();
    QString userBuildSupportFilePath();
    QString globalBuildSupportFilePath();
    QString supportBuildSystem(const QString &path);
    void createNewFileAction(QMenu *menu, const QString &path);
    void createNewFolderAction(QMenu *menu, const QString &path);
    void createMoveToTrash(QMenu *menu, const QString &path);
    void createDeleteAction(QMenu *menu, const QString &path);
    void createBuildAction(QMenu *menu, const QString &path);
};

TreeMenu::TreeMenu(QWidget *parent)
    : QMenu(parent)
    , d(new TreeMenuPrivate)
{
    d->initBuildSupport();
}

TreeMenu::~TreeMenu()
{
    if(d)
        delete d;
}

void TreeMenu::createNewFileAction(const QString &path)
{
    d->createNewFileAction(this, path);
}

void TreeMenu::createNewFolderAction(const QString &path)
{
    d->createNewFolderAction(this, path);
}

void TreeMenu::createMoveToTrash(const QString &path)
{
    d->createMoveToTrash(this, path);
}

void TreeMenu::createDeleteAction(const QString &path)
{
    d->createDeleteAction(this, path);
}

void TreeMenu::createBuildAction(const QString &path)
{
    d->createBuildAction(this, path);
}

void TreeMenuPrivate::initBuildSupport() {

    QString globalConfigBuildFile = globalBuildSupportFilePath();

    QFile globalFile(globalConfigBuildFile);
    if (!globalFile.exists()) {
        qCritical() << "Failed, not found global build menu config file";
        abort();
    }

    if (globalFile.open(QFile::ReadOnly)) {
        globalJsonDocument = QJsonDocument::fromJson(globalFile.readAll());
        globalFile.close();
    } else {
        qCritical() << "Failed, can't open global build menu config file";
        abort();
    }

    QString appConfigLocation = CustomPaths::user(CustomPaths::Configures);
    if (!QDir(appConfigLocation).exists()) {
        QDir().mkdir(appConfigLocation); //创建缓存目录
    }

    QString appConfigBuildSupportFile = appConfigLocation + QDir::separator() + BUILD_SUPPORT_FILE_NAME;
    QFileInfo fileInfo(appConfigBuildSupportFile);
    if (!fileInfo.exists()) {
        QFile::copy(globalConfigBuildFile, fileInfo.filePath());
    }

    QFile cachefile(appConfigBuildSupportFile);
    if (!cachefile.permissions().testFlag(QFile::WriteUser)) {
        cachefile.setPermissions(cachefile.permissions() | QFile::WriteUser);
    }

    if (cachefile.open(QFile::OpenModeFlag::ReadOnly)) {
        cacheJsonDocument = QJsonDocument::fromJson(cachefile.readAll());
        qInfo() << "cacheJsonDocument" << cacheJsonDocument;
        cachefile.close();
    } else {
        qCritical() << "Failed, can't open cache build menu config file";
        abort();
    }
}

QString TreeMenuPrivate::userBuildSupportFilePath()
{
    return CustomPaths::user(CustomPaths::Configures);
}

QString TreeMenuPrivate::globalBuildSupportFilePath()
{
    return CustomPaths::global(CustomPaths::Configures)
            + QDir::separator() + BUILD_SUPPORT_FILE_NAME;
}

QString TreeMenuPrivate::supportBuildSystem(const QString &path)
{
    QFileInfo fileInfo(path);
    QJsonObject globalJsonObj = globalJsonDocument.object();
    QStringList globalJsonObjKeys = globalJsonObj.keys();
    foreach (auto val, globalJsonObjKeys) {
        if (globalJsonObj.value(val).toObject().value("suffix").toArray().contains(fileInfo.suffix()))
            return val;
        if (globalJsonObj.value(val).toObject().value("base").toArray().contains(fileInfo.fileName()))
            return val;
    }

    QJsonObject cacheJsonObj = cacheJsonDocument.object();
    QStringList cacheJsonObjKeys = cacheJsonObj.keys();
    foreach (auto val, cacheJsonObjKeys) {
        if (cacheJsonObj.value(val).toObject().value("suffix").toArray().contains(fileInfo.suffix()))
            return val;
        if (cacheJsonObj.value(val).toObject().value("base").toArray().contains(fileInfo.fileName()))
            return val;
    }

    return "";
}

void TreeMenuPrivate::createNewFileAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;
    QAction *newAction = new QAction(NEW_FILE, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!parentInfo.isWritable()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, [=](){
        TreeProxy::instance().createFile(path);
    });
}

void TreeMenuPrivate::createNewFolderAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;
    QAction *newAction = new QAction(NEW_FOLDER, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!parentInfo.isWritable()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, newAction, [=](){
        TreeProxy::instance().createFolder(path);
    });
}

void TreeMenuPrivate::createMoveToTrash(QMenu *menu, const QString &path)
{
    if (!menu)
        return;
    QAction *newAction = new QAction(MOVE_TO_TARSH, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!info.isWritable() || !parentInfo.isWritable() || !ProcessUtil::hasGio()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, [=](){
        qInfo() << ProcessUtil::moveToTrash(path);
    });
}

void TreeMenuPrivate::createDeleteAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;

    menu->addSeparator();

    QAction *newAction = new QAction(DELETE, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());

    if (!info.isWritable() || !parentInfo.isWritable()) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered, [&](){
        if (QFileInfo(path).isDir())
            TreeProxy::instance().deleteFolder(path);
        if (QFileInfo(path).isFile())
            TreeProxy::instance().deleteFile(path);
    });
}

void TreeMenuPrivate::createBuildAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;

    menu->addSeparator();
    QFileInfo info(path);
    if (!info.isFile())
        return;

    QString buildSystem = supportBuildSystem(path);
    if (!buildSystem.isEmpty()) {
        QAction *newAction = new QAction(BUILD, menu);
        menu->addAction(newAction);
        QObject::connect(newAction, &QAction::triggered, [=](){
            SendEvents::buildProject(buildSystem,
                                     info.path() + QDir::separator() + "build",
                                     path, {});
        });
    }
}
