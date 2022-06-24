#include "mavenasynparse.h"
#include "mavenitemkeeper.h"
#include "services/project/projectgenerator.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>
#include <QtXml>

namespace  {

enum_def(CDT_XML_KEY, QString)
{
    enum_exp projectDescription = "projectDescription";
    enum_exp name = "name";
    enum_exp comment = "comment";
    enum_exp project = "project";
    enum_exp buildSpec = "buildSpec";
    enum_exp buildCommand = "buildCommand";
    enum_exp triggers = "triggers";
    enum_exp arguments = "arguments";
    enum_exp dictionary = "dictionary";
    enum_exp link = "link";
    enum_exp type = "type";
    enum_exp location = "location";
    enum_exp locationURI = "locationURI";
    enum_exp key = "key";
    enum_exp value = "value";
    enum_exp natures = "natures";
    enum_exp linkedResources = "linkedResources";
};

enum_def(CDT_TARGETS_TYPE, QString)
{
    enum_exp Subprojects = "[Subprojects]";
    enum_exp Targets = "[Targets]";
    enum_exp Lib = "[lib]";
    enum_exp Exe = "[exe]";
};

enum_def(CDT_FILES_TYPE, QString)
{
    enum_exp ObjectLibraries = "Object Libraries";
    enum_exp ObjectFiles = "Object Files";
    enum_exp SourceFiles = "Source Files";
    enum_exp HeaderFiles = "Header Files";
    enum_exp CMakeRules = "CMake Rules";
    enum_exp Resources = "Resources";
};

enum_def(CDT_CPROJECT_ATTR_KEY, QString)
{
    enum_exp moduleId = "moduleId";
    enum_exp id = "id";
    enum_exp name = "name";
    enum_exp path = "path";
    enum_exp targetID = "targetID";
};

// noly selection build targets
enum_def(CDT_MODULE_ID_VAL, QString)
{
    enum_exp org_eclipse_cdt_make_core_buildtargets = "org.eclipse.cdt.make.core.buildtargets";
};

QHash<QString, QString> optionHash {
    { "-S", "source directory" },
    { "-B", "build directory" },
    { "-G", "build system generator" },
    { "-DCMAKE_EXPORT_COMPILE_COMMANDS", "build clangd use compile json file"},
    { "-DCMAKE_BUILD_TYPE", "build type"}
};

QIcon cmakeFolderIcon()
{
    static QIcon cmakeFolderIcon;
    if (cmakeFolderIcon.isNull()) {
        cmakeFolderIcon = CustomIcons::icon(QFileIconProvider::Folder);
        cmakeFolderIcon.addFile(":/cmakeproject/images/fileoverlay_cmake@2x.png");
    }
    return cmakeFolderIcon;
}

QIcon libBuildIcon() {
    static QIcon libBuildIcon;
    if (libBuildIcon.isNull()) {
        libBuildIcon = CustomIcons::icon(CustomIcons::Lib);
        libBuildIcon.addFile(":/cmakeproject/images/build@2x.png");
    }
    return libBuildIcon;
}

QIcon exeBuildIcon()
{
    static QIcon exeBuildIcon;
    if (exeBuildIcon.isNull()) {
        exeBuildIcon = CustomIcons::icon(CustomIcons::Exe);
        exeBuildIcon.addFile(":/cmakeproject/images/build@2x.png");
    }
    return exeBuildIcon;
}

} // namespace

class MavenAsynParsePrivate
{
    friend  class MavenAsynParse;
    QThread *thread {nullptr};
    QDomDocument xmlDoc;
    QFileSystemWatcher watcher;
};

MavenAsynParse::MavenAsynParse()
    : d(new MavenAsynParsePrivate)
{
    d->thread = new QThread();
    this->moveToThread(d->thread);
}

MavenAsynParse::~MavenAsynParse()
{
    if (d) {
        if (d->thread) {
            if (d->thread->isRunning())
                d->thread->quit();
            while (d->thread->isFinished());
            delete d->thread;
            d->thread = nullptr;
        }
        delete d;
    }
}

void MavenAsynParse::loadPoms(const dpfservice::ProjectInfo &info)
{
    QFile docFile(info.projectFilePath());

    if (!docFile.exists()) {
        parsedError({"Failed, maven pro not exists!: " + docFile.fileName(), false});
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {;
        parsedError({docFile.errorString(), false});
    }

    if (!d->xmlDoc.setContent(&docFile)) {
        docFile.close();
    }
    docFile.close();
}

void MavenAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    ParseInfo<QList<QStandardItem*>> itemsResult;
    itemsResult.result = generatorChildItem(info.sourceFolder());
    itemsResult.isNormal = true;
    emit parsedProject(itemsResult);
}

void MavenAsynParse::parseActions(const QStandardItem *item)
{

}

bool MavenAsynParse::isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role) const
{
    if (!t1 || !t2)
        return false;
    return t1->data(role) == t2->data(role);
}

QList<QStandardItem *> MavenAsynParse::generatorChildItem(const QString &path) const
{
    QString rootPath = path;
    if (rootPath.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        rootPath = rootPath.remove(rootPath.size() - separatorSize, separatorSize);
    }

    QList<QStandardItem *> result;

    {// 避免变量冲突 迭代文件夹
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        dir.setSorting(QDir::Name);
        QDirIterator dirItera(dir, QDirIterator::Subdirectories);
        while (dirItera.hasNext()) {
            QString childPath = dirItera.next().remove(0, rootPath.size());
            QStandardItem *item = findItem(result, childPath);
            QIcon icon = CustomIcons::icon(dirItera.fileInfo());
            if (!item) {
                result << new QStandardItem(icon, dirItera.fileName());
            } else {
                item->appendRow(new QStandardItem(icon, dirItera.fileName()));
            }
        }
    }
    {// 避免变量冲突 迭代文件
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
        dir.setSorting(QDir::Name);
        QDirIterator fileItera(dir, QDirIterator::Subdirectories);
        while (fileItera.hasNext()) {
            QString childPath = fileItera.next().remove(0, rootPath.size());
            QStandardItem *item = findItem(result, childPath);
            QIcon icon = CustomIcons::icon(fileItera.fileInfo());
            if (!item) {
                result << new QStandardItem(icon, fileItera.fileName());
            } else {
                item->appendRow(new QStandardItem(icon, fileItera.fileName()));
            }
        }
    }
    return result;
}

QString MavenAsynParse::path(QStandardItem *item) const
{
    if (!item)
        return "";

    QStandardItem *currItem = item;
    QString result = item->data(Qt::DisplayRole).toString();
    while (currItem->parent()) {
        result.insert(0, QDir::separator());
        result.insert(0, path(currItem->parent()));
    }
    return result;
}

QList<QStandardItem *> MavenAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

QStandardItem *MavenAsynParse::findItem(QList<QStandardItem *> rowList, QString &path, QStandardItem *parent) const
{
    if (path.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        path = path.remove(path.size() - separatorSize, separatorSize);
    }

    for (int i = 0; i < rowList.size(); i++) {
        QString pathSplit = QDir::separator() + rowList[i]->data(Qt::DisplayRole).toString();
        if (path.startsWith(pathSplit)) {
            path = path.remove(0, pathSplit.size());
            return findItem(rows(rowList[i]), path, rowList[i]);
        }
    }

    return parent;
}
