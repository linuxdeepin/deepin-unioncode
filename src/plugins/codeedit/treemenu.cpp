#include "treemenu.h"
#include "treeproxy.h"

#include <QFileInfo>
#include <QAction>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QAbstractButton>

const QString NEW_FILE = "New File";
const QString NEW_FOLDER = "New Folder";
const QString MOVE_TO_TARSH = "Move To Trash";
const QString DELETE = "Delete";
const QString BUILD = "Build";

const QString NEW_FILE_NAME = "newCreateFile.txt";
const QString NEW_FOLDER_NAME = "newCreateFolder";

class TreeMenuPrivate
{
    friend class TreeMenu;
    QStringList buildScriptName {"CMakeLists.txt", "cmakelists.txt"};
    bool checkGlobalProcess(const QString &name);
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

}

TreeMenu::~TreeMenu()
{
    if(d) delete d;
}

void TreeMenu::createOptions(const QString &path)
{
    d->createNewFileAction(this, path);
    d->createNewFolderAction(this, path);
    d->createMoveToTrash(this, path);
    d->createDeleteAction(this, path);
    d->createBuildAction(this, path);
}

bool TreeMenuPrivate::checkGlobalProcess(const QString &name)
{
    bool ret = false;
    QProcess process;
    process.setProgram("whereis");
    process.setArguments({name});
    QProcess::connect(&process, &QProcess::readyRead, [&ret, &process, &name](){
        QList<QByteArray> rmSearch = process.readAll().split(' ');
        foreach (QByteArray rmProcess, rmSearch) {
            QFileInfo info(rmProcess);
            if (info.fileName() == name && info.isExecutable()) {
                if (!ret) ret = true;
            }
        }
    });
    process.start();
    process.waitForFinished();
    return ret;
}

void TreeMenuPrivate::createNewFileAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;
    QAction *newAction = new QAction(NEW_FILE, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!parentInfo.permissions().testFlag(QFile::WriteUser)
            || !checkGlobalProcess("touch")) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered,
                     [=](){
        QFileInfo info(path);
        QFileInfo parentInfo(info.path());
        int index = 1;
        QString newFileName = parentInfo.filePath() + "/" + NEW_FILE_NAME;
        QFileInfo createFileInfo(newFileName);
        while(createFileInfo.exists()) {
            createFileInfo.setFile(newFileName + QString("(%0)").arg(index));
            index ++;
        }
        QProcess process;
        process.setProgram("touch");
        process.setArguments({createFileInfo.filePath()});
        process.start();
        process.waitForFinished();
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
    if (!parentInfo.permissions().testFlag(QFile::WriteUser)
            || !checkGlobalProcess("mkdir")) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered,
                     [=](){
        QFileInfo info(path);
        QFileInfo parentInfo(info.path());
        int index = 1;
        QString newFolderName = parentInfo.filePath() + "/" + NEW_FOLDER_NAME;
        QFileInfo createFolderInfo(newFolderName);
        while(createFolderInfo.exists()) {
            createFolderInfo.setFile(newFolderName + QString("(%0)").arg(index));
            index ++;
        }
        QProcess process;
        process.setProgram("mkdir");
        process.setArguments({createFolderInfo.filePath()});
        process.start();
        process.waitForFinished();
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
    if (!info.permissions().testFlag(QFile::WriteUser)
            || !parentInfo.permissions().testFlag(QFile::WriteUser)
            || !checkGlobalProcess("gio")) {
        newAction->setEnabled(false);
        return;
    }

    QObject::connect(newAction, &QAction::triggered,
                     [&](){
        QProcess process;
        process.setProgram("gio");
        process.setArguments({"trash", path});
        process.start();
        process.waitForFinished();
    });
}

void TreeMenuPrivate::createDeleteAction(QMenu *menu, const QString &path)
{
    if (!menu)
        return;    void setPathIsFilesChild();
    void setPathIsFolder();

    menu->addSeparator();

    QAction *newAction = new QAction(DELETE, menu);
    menu->addAction(newAction);

    QFileInfo info(path);
    QFileInfo parentInfo(info.path());
    if (!info.permissions().testFlag(QFile::Permission::WriteUser)
            || !parentInfo.permissions().testFlag(QFile::WriteUser)
            || !checkGlobalProcess("rm")) {
        newAction->setEnabled(false);    void setPathIsFilesChild();
        void setPathIsFolder();
        return;
    }

    QObject::connect(newAction, &QAction::triggered,
                     [&](){
        QMessageBox warning;
        warning.setWindowTitle("Delete Warning");
        warning.setText("The delete operation will be removed from"
                        "the disk and will not be recoverable "
                        "after this operation.\nDelete anyway?");
        warning.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
        QObject::connect(warning.button(QMessageBox::Ok), &QAbstractButton::clicked,
                         [=](){
            QProcess process;
            process.setProgram("rm");
            process.setArguments({"-r", path});
            process.start();
            process.waitForFinished();
        });
        warning.exec();
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

    if (buildScriptName.contains(info.fileName())) {
        QAction *newAction = new QAction(BUILD, menu);
        menu->addAction(newAction);
    }
}
