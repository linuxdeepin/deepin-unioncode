#include "cmakegenerator.h"

namespace  {
enum_def(ProjectKit, QString)
{
    enum_exp CDT = "Eclipse CDT4 - Unix Makefiles";
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
    enum_exp TargetsLib = Targets + "/" + Lib;
    enum_exp TargetsExe = Targets + "/" + Exe;
};

enum_def(CDT_FILES_TYPE, QString)
{
    enum_exp Unknown = "//";
    enum_exp ObjectLibraries = "/Object Libraries/";
    enum_exp ObjectFiles = "/Object Files/";
    enum_exp SourceFiles = "/Source Files/";
    enum_exp HeaderFiles = "/Header Files/";
    enum_exp CMakeRules = "/CMake Rules/";
    enum_exp Resources = "/Resources/";
};

static int currentCount = 0;
static int maxCount = 100;
}

CMakeGenerator::CMakeGenerator()
{

}

QStandardItem *CMakeGenerator::createRootItem(const QString &projectPath)
{
    Generator::started();
    currentCount = 0;
    maxCount = 100;

    QFileInfo cmakeFileInfo(projectPath);
    QString cmakeFilePath(cmakeFileInfo.path());
    QString cmakeBuildPath = cmakeFilePath + QDir::separator() + "build";

    process.setProgram("cmake");

    QStringList arguments;
    arguments << "-S";
    arguments << cmakeFilePath;
    arguments << "-B";
    arguments << cmakeBuildPath;
    arguments << "-G";
    arguments << ProjectKit::get()->CDT;
    arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
    process.setArguments(arguments);

    // 消息和進度轉發
    QObject::connect(&process, &QProcess::readyRead,
                     this, &CMakeGenerator::processReadAll, Qt::UniqueConnection);

    QObject::connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this , &CMakeGenerator::processFinished, Qt::UniqueConnection);
    process.start();

    QStandardItem * rootItem = new QStandardItem("Test Root");
    return rootItem;
}

QMenu *CMakeGenerator::createIndexMenu(const QModelIndex &index)
{
    Q_UNUSED(index);
    return nullptr;
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
    mess += QString("return code: %0").arg(code) + QString(mateEnum.key(status));
    qInfo() << mess;
    message({mess, maxCount, maxCount});
}
