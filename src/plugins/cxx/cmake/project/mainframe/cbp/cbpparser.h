// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QXmlStreamReader>

#include <memory>

enum class FileType : quint16 {
    Unknown = 0,
    Header,
    Source,
    Form,
    StateChart,
    Resource,
    QML,
    Project,
    FileTypeSize
};

class ProjectFile
{
public:
    ProjectFile(const QString &_filePath, const FileType _fileType, bool _generated)
        : fileType(_fileType),
          filePath(_filePath),
          generated(_generated)
    {
    }

    FileType getFileType() const;
    const QString &getfilePath() const
    {
        return filePath;
    }

private:
    FileType fileType;
    QString filePath;
    bool generated;
};

enum CBPTargetType {
    kExecutable = 0,
    kStaticLibrary = 2,
    kDynamicLibrary = 3,
    kUtility = 64
};

struct CMakeBuildTarget
{
public:
    QString title;
    QString output;
    CBPTargetType type = kUtility;
    QString workingDirectory;
    QString sourceDirectory;
    QString makeCommand;

    // code model
    QList<QString> includeFiles;
    QStringList compilerOptions;
    QList<QString> srcfiles;

    void clear()
    {
        output.clear();
        makeCommand.clear();
        workingDirectory.clear();
        sourceDirectory.clear();
        title.clear();
        type = kUtility;
        includeFiles.clear();
        compilerOptions.clear();
        srcfiles.clear();
    }
};
Q_DECLARE_METATYPE(CMakeBuildTarget)

class CMakeCbpParser : public QXmlStreamReader
{
public:
    bool parseCbpFile(const QString &fileName,
                      const QString &sourceDirectory);
    std::vector<std::unique_ptr<ProjectFile>> &&
    getTakeFileList() { return std::move(srcFileList); }
    std::vector<std::unique_ptr<ProjectFile>> &&
    getCmakeFileList() { return std::move(cmakeFileList); }
    const QList<CMakeBuildTarget> &getBuildTargets() const;
    QString getProjectName() const;
    QString getCompilerName() const;
    bool hasCMakeFiles();

private:
    void parseCodeBlocks_project_file();
    void parseProject();
    void parseBuild();
    void parseOption();
    void parseBuildTarget();
    void parseBuildTargetOption();
    void parseMakeCommands();
    void parseBuildTargetBuild();
    void parseBuildTargetClean();
    void parseCompiler();
    void parseAdd();
    void parseUnit();
    void parseUnitOption();
    void parseUnknownElement();
    void sortFiles();

    QMap<QString, QStringList> unitTargetMap;
    std::vector<std::unique_ptr<ProjectFile>> srcFileList;
    std::vector<std::unique_ptr<ProjectFile>> cmakeFileList;
    QSet<QString> processedUnits;
    bool parsingCMakeUnit = false;

    CMakeBuildTarget buildTarget;
    QList<CMakeBuildTarget> buildTargets;
    QString projectName;
    QString compiler;
    QString sourceDirectory;
    QString buildDirectory;
    QStringList unitTargets;
};
