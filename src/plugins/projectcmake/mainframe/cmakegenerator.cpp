#include "cmakegenerator.h"
#include "cmakeconfigwidget.h"
#include <QtXml>
#include <QFileIconProvider>

namespace  {

enum_def(CDT_PROJECT_KIT, QString)
{
    enum_exp CDT4_GENERATOR = "Eclipse CDT4 - Unix Makefiles";
    enum_exp PROJECT_FILE = ".project";
    enum_exp CPROJECT_FILE = ".cproject";
    enum_exp CMAKE_BUILD_PATH = "buildPath";
    enum_exp CMAKE_SOURCE_PATH = "sourcePath";
    enum_exp CMAKE_FILE_PATH = "cmakeFilePath";
};

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

static int currentCount = 0;
static int maxCount = 100;

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

}

CMakeGenerator::CMakeGenerator()
{

}

bool CMakeGenerator::configure(const QString &projectPath)
{
    Q_UNUSED(projectPath);
    CMakeConfigWidget configDialog;
    configDialog.show();
    return true;
}

QStandardItem *CMakeGenerator::createRootItem(const QString &projectPath)
{
    Generator::started();
    currentCount = 0;
    maxCount = 100;

    QString buildPath = cmakeBuildPath(projectPath);
    QString sourcePath = QFileInfo(projectPath).path();
    process.setProgram("cmake");

    QStringList arguments;
    arguments << "-S";
    arguments << sourcePath;
    arguments << "-B";
    arguments << buildPath;
    arguments << "-G";
    arguments << CDT_PROJECT_KIT::get()->CDT4_GENERATOR;
    arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
    arguments << "-DCMAKE_BUILD_TYPE=\"Release\"";
    process.setArguments(arguments);

    // 消息和進度轉發
    QObject::connect(&process, &QProcess::readyRead,
                     this, &CMakeGenerator::processReadAll, Qt::UniqueConnection);

    QObject::connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this , &CMakeGenerator::processFinished, Qt::UniqueConnection);
    process.start();

    // step.1 cmake generator CDT4 project and compile json file
    if (process.exitCode() != 0 || QProcess::ExitStatus::NormalExit) {
        Generator::setErrorString(process.errorString());
        Generator::finished(false);
        return nullptr; // nullptr root item
    }

    // step.2 read project from CDT4 xml file;
    QStandardItem * rootItem = nullptr;
    auto projectXmlDoc = cdt4LoadProjectXmlDoc(projectPath);
    QDomElement docElem = projectXmlDoc.documentElement();
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if (!rootItem && e.tagName() == CDT_XML_KEY::get()->name) {
                QFileInfo rootCMakeInfo(projectPath);
                rootItem = new QStandardItem();
                // 设置顶层节点当前构建系统信息，该过程不可少
                ProjectGenerator::setToolKitName(rootItem, CMakeGenerator::toolKitName());
                ProjectGenerator::setToolKitProperty(rootItem, CDT_PROJECT_KIT::get()->CMAKE_BUILD_PATH, buildPath);
                ProjectGenerator::setToolKitProperty(rootItem, CDT_PROJECT_KIT::get()->CMAKE_SOURCE_PATH, sourcePath);
                ProjectGenerator::setToolKitProperty(rootItem, CDT_PROJECT_KIT::get()->CMAKE_FILE_PATH, projectPath);
                rootItem->setText(e.text());
                rootItem->setIcon(::cmakeFolderIcon());
                rootItem->setToolTip(rootCMakeInfo.dir().path());
                // 添加子集cmake文件
                auto rootCMakeItem = new QStandardItem();
                rootCMakeItem->setText(rootCMakeInfo.fileName());
                rootCMakeItem->setIcon(CustomIcons::icon(rootCMakeInfo));
                rootCMakeItem->setToolTip(rootCMakeInfo.filePath());
                rootItem->appendRow(rootCMakeItem);
            }
            if (rootItem && e.tagName() == CDT_XML_KEY::get()->linkedResources) {
                QDomNode linkedResChildNode = e.firstChild();
                while (!linkedResChildNode.isNull()) {
                    QDomElement linkElem = linkedResChildNode.toElement();
                    if (linkElem.tagName() == CDT_XML_KEY::get()->link) {
                        QDomNode linkChildNode = linkElem.firstChild();
                        QStandardItem *childItem = nullptr;
                        while (!linkChildNode.isNull()) {
                            QDomElement linkChildElem = linkChildNode.toElement();
                            if (!childItem && linkChildElem.tagName() == CDT_XML_KEY::get()->name) {
                                QString name = linkChildElem.text();
                                auto parentItem = cdt4FindParentItem(rootItem, name); // 查找上级节点
                                if (!name.isEmpty()) { // 节点为有效数据 避免 dir/ 和dir解析歧义生成空节点
                                    childItem = new QStandardItem(); // 创建子节点
                                    childItem->setText(name); // 设置子节点名称
                                    if (parentItem) {
                                        parentItem->appendRow(childItem);
                                    }
                                }
                            }
                            if (childItem && linkChildElem.tagName() == CDT_XML_KEY::get()->location) {
                                childItem->setIcon(CustomIcons::icon(QFileInfo(linkChildElem.text()))); // 设置本地文件图标
                                childItem->setToolTip(linkChildElem.text());
                            }
                            if (childItem && linkChildElem.tagName() == CDT_XML_KEY::get()->locationURI) {
                                childItem->setToolTip(linkChildElem.text());
                            }
                            // 节点缓存来自xml的所有数据
                            ProjectGenerator::setToolKitProperty(childItem, linkChildElem.tagName(), linkChildElem.text());
                            linkChildNode = linkChildNode.nextSibling();
                        }
                    }
                    linkedResChildNode = linkedResChildNode.nextSibling();
                }
            }
        }
        n = n.nextSibling();
    }
    return cdt4DisplayOptimize(rootItem);
}

QMenu *CMakeGenerator::createItemMenu(const QStandardItem *item)
{
    // 读取文件
    auto rootItem = ProjectGenerator::top(item);

    QString buildPath = ProjectGenerator::toolKitProperty
            (rootItem, CDT_PROJECT_KIT::get()->CMAKE_BUILD_PATH).toString();

    QDomDocument doc = cdt4LoadMenuXmlDoc(cdt4FilePath(buildPath));

    QMenu *result = new QMenu;
    result->addAction(new QAction("Test"));
    return result;
}

void CMakeGenerator::processReadAll()
{
    QString mess = process.readAll();
    currentCount += 10;
    qInfo() << mess;
    message({mess, currentCount, maxCount});
}

void CMakeGenerator::processFinished(int code, QProcess::ExitStatus status)
{
    QString mess = process.readAll();
    QMetaEnum mateEnum = QMetaEnum::fromType<QProcess::ExitStatus>();
    mess += "\n";
    mess += QString("return code: %0, return Status: %1").arg(code).arg(mateEnum.key(status));
    qInfo() << mess;
    message({mess, maxCount, maxCount});
}

void CMakeGenerator::cdt4SubprojectsDisplayOptimize(QStandardItem *item)
{
    if (!item) {
        return;
    }
    for (int row = 0; row < item->rowCount(); row ++) {
        QStandardItem *childItem = item->child(row);
        QString location = ProjectGenerator::toolKitProperty(childItem, CDT_XML_KEY::get()->location).toString();
        QFileInfo cmakeFileInfo(location, "CMakeLists.txt");
        if (cmakeFileInfo.exists()) {
           auto cmakeFileItem = new QStandardItem();
           cmakeFileItem->setIcon(CustomIcons::icon(cmakeFileInfo));
           cmakeFileItem->setText(cmakeFileInfo.fileName());
           cmakeFileItem->setToolTip(cmakeFileInfo.filePath());
           childItem->appendRow(cmakeFileItem);
        }
    }
}

QStandardItem *CMakeGenerator::cdt4FindParentItem(QStandardItem *rootItem, QString &name)
{
    if (!rootItem) {
        return nullptr;
    }
    for (int row = 0; row < rootItem->rowCount(); row ++) {
        QStandardItem *childItem = rootItem->child(row);
        QString childDisplayText = childItem->data(Qt::DisplayRole).toString();
        if (name.startsWith(childDisplayText + "/")) {
            name = name.replace(childDisplayText + "/", "");
            return cdt4FindParentItem(childItem, name);
        }
    }
    return rootItem;
}

QStandardItem *CMakeGenerator::cdt4FindItem(QStandardItem *rootItem, QString &name)
{
    if (!rootItem) {
        return nullptr;
    }

    QStandardItem *parentItem = cdt4FindParentItem(rootItem, name);
    if (parentItem) {
        for (int row = 0; row < parentItem->rowCount(); row ++) {
            QStandardItem * childItem = parentItem->child(row);
            qInfo() << parentItem->data(Qt::DisplayRole) << childItem->data(Qt::DisplayRole);
            if (childItem->data(Qt::DisplayRole) == name) {
                name = name.replace(childItem->data(Qt::DisplayRole).toString(), "");
                return childItem;
            }
        }
    }
    return parentItem;
}

QHash<QString, QString> CMakeGenerator::cdt4Subporjects(QStandardItem *rootItem)
{
    QString subprojectsKey = CDT_TARGETS_TYPE::get()->Subprojects;
    QStandardItem * subprojectsItem = cdt4FindItem(rootItem, subprojectsKey);
    QHash<QString, QString> subprojectHash;
    for (int row = 0; row < subprojectsItem->rowCount(); row ++) {
        auto name = ProjectGenerator::toolKitProperty(subprojectsItem->child(row), CDT_XML_KEY::get()->name).toString();
        auto location = ProjectGenerator::toolKitProperty(subprojectsItem->child(row), CDT_XML_KEY::get()->location).toString();
        subprojectHash[name] = location;
    }
    return subprojectHash;
}

QStandardItem *CMakeGenerator::cdt4DisplayOptimize(QStandardItem *rootItem)
{
    if (!rootItem) {
        return nullptr;
    }

    // 优化二级目录节点
    for (int row = 0; row < rootItem->rowCount(); row ++) {
        QStandardItem *childItem = rootItem->child(row);
        QString displayName = childItem->data(Qt::DisplayRole).toString();
        // cmake folder setting
        if (displayName.contains(CDT_TARGETS_TYPE::get()->Targets)
                || displayName.contains(CDT_TARGETS_TYPE::get()->Subprojects)) {
            childItem->setIcon(::cmakeFolderIcon());
        }
    }

    // 优化subprojects及子节点
    QString subprojectsName = CDT_TARGETS_TYPE::get()->Subprojects;
    QStandardItem *subprojectItem = cdt4FindItem(rootItem, subprojectsName);
    cdt4SubprojectsDisplayOptimize(subprojectItem);

    // 优化targets及子节点
    QString targetsName = CDT_TARGETS_TYPE::get()->Targets;
    QStandardItem *targetsItem = cdt4FindItem(rootItem, targetsName);
    QHash<QString, QString> subprojectsMap = cdt4Subporjects(rootItem);
    cdt4TargetsDisplayOptimize(targetsItem, subprojectsMap);
    return rootItem;
}

void CMakeGenerator::cdt4TargetsDisplayOptimize(QStandardItem *item, const QHash<QString, QString> &subprojectsMap)
{
    if (!item)
        return;

    QStandardItem * addRows = new QStandardItem("Temp");

    for (int row = 0; row < item->rowCount(); row ++) {
        QStandardItem *childItem = item->child(row);
        QString displayName = childItem->data(Qt::DisplayRole).toString();
        // build lib icon setting
        if (displayName.contains(CDT_TARGETS_TYPE::get()->Lib)) {
            childItem->setIcon(::libBuildIcon());
        }
        // build exe icon setting
        if (displayName.contains(CDT_TARGETS_TYPE::get()->Exe)) {
            childItem->setIcon(::exeBuildIcon());
        }
        // cmake folder setting
        for (int index = 0; index < CDT_FILES_TYPE::get()->count(); index ++) {
            if (CDT_FILES_TYPE::get()->value(index) == displayName) {
                childItem->setIcon(::cmakeFolderIcon());
            }
        }

        if (childItem->hasChildren()) {
            cdt4TargetsDisplayOptimize(item->child(row), subprojectsMap);
        } else {
            QVariantMap map = ProjectGenerator::toolKitPropertyMap(childItem); //当前节点特性
            if (map.keys().contains(CDT_XML_KEY::get()->location)) { // 本地文件
                for (auto val : subprojectsMap.values()) {
                    QString childLocation = map.value(CDT_XML_KEY::get()->location).toString();
                    qInfo() << "childLocation:" << childLocation;
                    QString childFileName = childItem->data(Qt::DisplayRole).toString();
                    // 获取中间需要展示的文件夹
                    if (!val.isEmpty() && childLocation.startsWith(val)) {
                        QString suffixPath = childLocation.replace(val + "/","");
                        if (suffixPath.endsWith("/" + childFileName)) {
                            suffixPath = suffixPath.remove(suffixPath.size() - childFileName.size() - 1,
                                                           childFileName.size() + 1);
                            // 获取当前是否已经新建文件夹
                            QStandardItem *findNewItem = cdt4FindItem(addRows, suffixPath);
                            if (!suffixPath.isEmpty()) { // 新建子文件夹
                                QIcon icon = CustomIcons::icon(QFileIconProvider::Folder);
                                auto newChild = new QStandardItem(icon, suffixPath);
                                findNewItem->insertRow(0, newChild); //置顶文件夹
                                findNewItem = newChild;
                            }
                            // 当前子节点移动到找到的节点下
                            qInfo() << item->rowCount();
                            findNewItem->appendRow(item->takeRow(row));
                            row --; //takeRow自动删除一行，此处屏蔽差异
                            qInfo() << item->rowCount();
                            qInfo() << findNewItem->data(Qt::DisplayRole).toString();
                            qInfo() << findNewItem->parent()->data(Qt::DisplayRole).toString();
                        }
                    }
                }
            }
        }
    }
    // 新增项
    while (addRows->hasChildren()) {
        QStandardItem *addRowItem = addRows->takeRow(0).first();
        qInfo() << addRowItem->data(Qt::DisplayRole).toString();
        item->appendRow(addRowItem);
    }
    delete addRows;
}

QDomDocument CMakeGenerator::cdt4LoadProjectXmlDoc(const QString &cmakePath)
{
    QDomDocument xmlDoc;
    QString cdtProjectFile = cdt4FilePath(cmakePath) + QDir::separator()
            + CDT_PROJECT_KIT::get()->PROJECT_FILE;
    QFile docFile(cdtProjectFile);

    if (!docFile.exists()) {
        Generator::setErrorString("Failed, cdtProjectFile not exists!: " + cdtProjectFile);
        Generator::finished(false);
        return xmlDoc;
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {
        Generator::setErrorString(docFile.errorString());
        Generator::finished(false);
        return xmlDoc;
    }

    if (!xmlDoc.setContent(&docFile)) {
        docFile.close();
        return xmlDoc;
    }
    docFile.close();
    return xmlDoc;
}

QDomDocument CMakeGenerator::cdt4LoadMenuXmlDoc(const QString &cmakePath)
{
    QDomDocument xmlDoc;
    QString cdtMenuFile = cdt4FilePath(cmakePath) + QDir::separator()
            + CDT_PROJECT_KIT::get()->CPROJECT_FILE;
    QFile docFile(cdtMenuFile);

    if (!docFile.exists()) {
        Generator::setErrorString("Failed, cdtMenuFile not exists!: " + cdtMenuFile);
        Generator::finished(false);
        return xmlDoc;
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {
        Generator::setErrorString(docFile.errorString());
        Generator::finished(false);
        return xmlDoc;
    }

    if (!xmlDoc.setContent(&docFile)) {
        docFile.close();
        return xmlDoc;
    }
    docFile.close();
    return xmlDoc;
}

QString CMakeGenerator::cdt4FilePath(const QString &cmakePath)
{
    return cmakeBuildPath(cmakePath);
}

QString CMakeGenerator::cmakeBuildPath(const QString &cmakePath)
{
    return QFileInfo(cmakePath).path() + QDir::separator() + "build";
}
