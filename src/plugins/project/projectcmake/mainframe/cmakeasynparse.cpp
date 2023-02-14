#include "cmakeasynparse.h"
#include "cmakeitemkeeper.h"
#include "services/project/projectgenerator.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>

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

CmakeAsynParse::CmakeAsynParse()
{

}

CmakeAsynParse::~CmakeAsynParse()
{
    qInfo() << __FUNCTION__;
}

QStandardItem *CmakeAsynParse::parseProject(QStandardItem *rootItem, const dpfservice::ProjectInfo &info)
{
    if (!rootItem)
        return nullptr;

    using namespace dpfservice;
    QSet<QString> allFiles{};
    ProjectInfo tempInfo = info;
    QDomDocument projectXmlDoc = cdt4LoadProjectXmlDoc(tempInfo.buildFolder());
    QDomElement docElem = projectXmlDoc.documentElement();
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        qInfo() << e.text();
        if(!e.isNull()) {
            if (e.tagName() == CDT_XML_KEY::get()->name) {

                rootItem->setText(QFileInfo(tempInfo.workspaceFolder()).fileName());
                rootItem->setIcon(::cmakeFolderIcon());
                rootItem->setToolTip(tempInfo.workspaceFolder());

                // 添加子集cmake文件
                QDir rootDir(tempInfo.workspaceFolder());
                QString cmakeItemName = "";
                if (rootDir.exists("CMakeLists.txt"))
                    cmakeItemName = "CMakeLists.txt";
                if (rootDir.exists("cmakelists.txt"))
                    cmakeItemName = "cmakelists.txt";
                if (!cmakeItemName.isEmpty()) {
                    auto rootCMakeItem = new QStandardItem();
                    rootCMakeItem->setText(cmakeItemName);
                    rootCMakeItem->setIcon(CustomIcons::icon(rootDir.filePath(cmakeItemName)));
                    rootCMakeItem->setToolTip(rootDir.filePath(cmakeItemName));
                    rootItem->appendRow(rootCMakeItem);
                    // 添加监听节点顶层工程路径
                    CmakeItemKeeper::instance()->addCmakeRootFile(rootItem, rootDir.filePath(cmakeItemName));
                }
            }
            if (e.tagName() == CDT_XML_KEY::get()->linkedResources) {
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
                                QFileInfo fileInfo(linkChildElem.text());
                                if (fileInfo.isFile()) {
                                    allFiles << linkChildElem.text();
                                }
                                childItem->setIcon(CustomIcons::icon(fileInfo)); // 设置本地文件图标
                                childItem->setToolTip(linkChildElem.text());
                            }
                            if (childItem && linkChildElem.tagName() == CDT_XML_KEY::get()->locationURI) {
                                childItem->setToolTip(linkChildElem.text());
                            }
                            // 节点缓存来自xml的所有数据
                            dpfservice::ProjectInfo::set(childItem, linkChildElem.tagName(), linkChildElem.text());
                            linkChildNode = linkChildNode.nextSibling();
                        }
                    }
                    linkedResChildNode = linkedResChildNode.nextSibling();
                }
            }
        }
        n = n.nextSibling();
    }

    // 设置顶层节点当前构建系统信息，该过程不可少
    tempInfo.setSourceFiles(allFiles);
    ProjectInfo::set(rootItem, tempInfo);
    rootItem = cdt4DisplayOptimize(rootItem);
    emit parseProjectEnd({rootItem, true});
    return rootItem;
}

QList<CmakeAsynParse::TargetBuild> CmakeAsynParse::parseActions(const QStandardItem *item)
{
    QList<TargetBuild> buildMenuList;

    if (!item)
        return {};

    QString itemName = dpfservice::ProjectInfo::get(item).property(CDT_XML_KEY::get()->name).toString();
    QString itemLocalURI = dpfservice::ProjectInfo::get(item).property(CDT_XML_KEY::get()->locationURI).toString();

    // 读取文件
    using namespace dpfservice;
    const QStandardItem *rootItem = ProjectGenerator::root(const_cast<QStandardItem*>(item));


    // 顶层文件存在 并且名称存在 并且是虚拟路径
    if (rootItem && !itemName.isEmpty() && !itemLocalURI.isEmpty()) {
        QString buildPath = dpfservice::ProjectInfo::get(rootItem).buildFolder();
        QDomDocument menuXmlDoc = cdt4LoadMenuXmlDoc(buildPath);
        QDomElement docElem = menuXmlDoc.documentElement();
        QDomNode n = docElem.firstChild().firstChild().firstChild(); // 过滤三层
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull() && e.tagName() == CDT_CPROJECT_KEY::get()->storageModuled
                    && e.attribute(CDT_CPROJECT_ATTR_KEY::get()->moduleId)
                    == CDT_MODULE_ID_VAL::get()->org_eclipse_cdt_make_core_buildtargets) {
                QDomNode targetNode = e.firstChild().firstChild(); // 过滤两层
                while (!targetNode.isNull()) {
                    QDomElement targetElem = targetNode.toElement();
                    if (targetElem.attribute(CDT_CPROJECT_ATTR_KEY::get()->path) == itemName) {
                        QDomNode targetBuildChild = targetElem.firstChild();
                        struct TargetBuild targetBuild;
                        targetBuild.buildName = targetElem.attribute(CDT_CPROJECT_ATTR_KEY::get()->name);
                        qInfo() << targetBuild.buildName;
                        while (!targetBuildChild.isNull()) {
                            auto targetBuildTargetElem = targetBuildChild.toElement();
                            if (targetBuildTargetElem.tagName() == CDT_CPROJECT_KEY::get()->buildCommand) {
                                targetBuild.buildCommand = targetBuildTargetElem.text();
                                qInfo() << targetBuild.buildCommand;
                            }
                            if (targetBuildTargetElem.tagName() == CDT_CPROJECT_KEY::get()->buildArguments) {
                                targetBuild.buildArguments = targetBuildTargetElem.text();
                                qInfo() << targetBuild.buildArguments;
                            }
                            if (targetBuildTargetElem.tagName() == CDT_CPROJECT_KEY::get()->buildTarget) {
                                targetBuild.buildTarget = targetBuildTargetElem.text();
                                qInfo() << targetBuild.buildTarget;
                            }
                            if (targetBuildTargetElem.tagName() == CDT_CPROJECT_KEY::get()->stopOnError) {
                                targetBuild.stopOnError = targetBuildTargetElem.text();
                                qInfo() << targetBuild.stopOnError;
                            }
                            if (targetBuildTargetElem.tagName() == CDT_CPROJECT_KEY::get()->useDefaultCommand) {
                                targetBuild.useDefaultCommand = targetBuildTargetElem.text();
                                qInfo() << targetBuild.useDefaultCommand;
                            }
                            targetBuildChild = targetBuildChild.nextSibling();
                        }
                        if (!targetBuild.isInvalid()) {
                            buildMenuList << targetBuild;
                        }
                    }
                    targetNode = targetNode.nextSibling();
                }
            }
            n = n.nextSibling();
        }
    }

    parseActionsEnd({buildMenuList, true});
    return buildMenuList;
}

QDomDocument CmakeAsynParse::cdt4LoadProjectXmlDoc(const QString &buildFolder)
{
    QDomDocument xmlDoc;
    QString cdtProjectFile = buildFolder + QDir::separator()
            + CDT_PROJECT_KIT::get()->PROJECT_FILE;
    QFile docFile(cdtProjectFile);

    if (!docFile.exists()) {
        parseError({"Failed, cdtProjectFile not exists!: " + cdtProjectFile, false});
        return xmlDoc;
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {;
        parseError({docFile.errorString(), false});
        return xmlDoc;
    }

    if (!xmlDoc.setContent(&docFile)) {
        docFile.close();
        return xmlDoc;
    }
    docFile.close();
    return xmlDoc;
}

void CmakeAsynParse::cdt4SubprojectsDisplayOptimize(QStandardItem *item)
{
    if (!item) {
        return;
    }
    using namespace dpfservice;
    QStringList subFiles;
    for (int row = 0; row < item->rowCount(); row ++) {
        QStandardItem *childItem = item->child(row);
        QString location = dpfservice::ProjectInfo::get(childItem).property(CDT_XML_KEY::get()->location).toString();
        QFileInfo cmakeFileInfo(location, "CMakeLists.txt");
        if (cmakeFileInfo.exists()) {
            auto cmakeFileItem = new QStandardItem();
            cmakeFileItem->setIcon(CustomIcons::icon(cmakeFileInfo));
            cmakeFileItem->setText(cmakeFileInfo.fileName());
            cmakeFileItem->setToolTip(cmakeFileInfo.filePath());
            subFiles << cmakeFileInfo.filePath();
            childItem->appendRow(cmakeFileItem);
        }
    }

    auto rootItem = ProjectGenerator::root(item);
    if (rootItem) {
        CmakeItemKeeper::instance()->addCmakeSubFiles(rootItem, subFiles);
    } else {
        qCritical() << "Failed, can't add sub project file to watcher";
    }
}

QStandardItem *CmakeAsynParse::cdt4FindParentItem(QStandardItem *rootItem, QString &name)
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

QStandardItem *CmakeAsynParse::cdt4FindItem(QStandardItem *rootItem, QString &name)
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

QHash<QString, QString> CmakeAsynParse::cdt4Subporjects(QStandardItem *rootItem)
{
    QString subprojectsKey = CDT_TARGETS_TYPE::get()->Subprojects;
    QStandardItem * subprojectsItem = cdt4FindItem(rootItem, subprojectsKey);
    QHash<QString, QString> subprojectHash;
    for (int row = 0; row < subprojectsItem->rowCount(); row ++) {
        auto name = dpfservice::ProjectInfo::get(subprojectsItem->child(row)).property(CDT_XML_KEY::get()->name).toString();
        auto location = dpfservice::ProjectInfo::get(subprojectsItem->child(row)).property(CDT_XML_KEY::get()->location).toString();
        subprojectHash[name] = location;
    }
    return subprojectHash;
}

QStandardItem *CmakeAsynParse::cdt4DisplayOptimize(QStandardItem *rootItem)
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

void CmakeAsynParse::cdt4TargetsDisplayOptimize(QStandardItem *item, const QHash<QString, QString> &subprojectsMap)
{
    if (!item)
        return;

    QStandardItem * addRows = new QStandardItem("Temp");
    for (int row = 0; row < item->rowCount(); ++row) {
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
            auto info = dpfservice::ProjectInfo::get(childItem); //当前节点特性
            if (info.hasKey(CDT_XML_KEY::get()->location)) { // 本地文件
                QString childLocation = info.property(CDT_XML_KEY::get()->location).toString();
                // qInfo() << "childLocation:" << childLocation;
                QString childFileName = childItem->data(Qt::DisplayRole).toString();
                QString prefixPath;
                QString suffixPath;
                for (auto val : subprojectsMap.values()) { // 获取中间需要展示的文件夹
                    if (!val.isEmpty() && childLocation.startsWith(val)) {
                        suffixPath = childLocation.replace(val + "/","");
                        if (suffixPath.endsWith("/" + childFileName)) {
                            suffixPath = suffixPath.remove(suffixPath.size() - childFileName.size() - 1,
                                                           childFileName.size() + 1);
                            prefixPath = val;
                        } else if (suffixPath == childFileName) {
                            suffixPath = "";
                        }
                    }
                }
                if (!suffixPath.isEmpty()) {
                    QString suffixPathTemp = suffixPath;
                    // 获取当前是否已经新建文件夹，此函数会处理 suffixPathTemp
                    QStandardItem *findNewItem = cdt4FindItem(addRows, suffixPathTemp);
                    if (!suffixPathTemp.isEmpty()) { // 新建子文件夹
                        QIcon icon = CustomIcons::icon(QFileIconProvider::Folder);
                        auto newChild = new QStandardItem(icon, suffixPathTemp);
                        newChild->setToolTip(prefixPath + QDir::separator() + suffixPath);
                        findNewItem->insertRow(0, newChild); //置顶文件夹
                        findNewItem = newChild;
                    }
                    // 当前子节点移动到找到的节点下
                    findNewItem->appendRow(item->takeRow(row));
                    --row;
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

QDomDocument CmakeAsynParse::cdt4LoadMenuXmlDoc(const QString &buildFolder)
{
    QDomDocument xmlDoc;
    QString cdtMenuFile = buildFolder + QDir::separator()
            + CDT_PROJECT_KIT::get()->CPROJECT_FILE;
    QFile docFile(cdtMenuFile);

    if (!docFile.exists()) {
        parseError({"Failed, cdtMenuFile not exists!: " + cdtMenuFile, false});
        return xmlDoc;
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {
        parseError({docFile.errorString(), false});
        return xmlDoc;
    }

    if (!xmlDoc.setContent(&docFile)) {
        docFile.close();
        return xmlDoc;
    }
    docFile.close();
    return xmlDoc;
}
