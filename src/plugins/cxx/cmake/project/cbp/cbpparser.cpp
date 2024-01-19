// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cbpparser.h"

#include <QDir>
#include <QTextStream>

using FileNameList = QList<QString>;

template<typename Container>
inline void sort(Container &container)
{
    std::sort(std::begin(container), std::end(container));
}

template<typename R, typename S, typename T>
decltype(auto) equal(R S::*member, T value)
{
    return std::bind<bool>(std::equal_to<T>(), value, std::bind(member, std::placeholders::_1));
}

template<typename C, typename F>
int indexOf(const C &container, F function)
{
    typename C::const_iterator begin = std::begin(container);
    typename C::const_iterator end = std::end(container);

    typename C::const_iterator it = std::find_if(begin, end, function);
    return it == end ? -1 : static_cast<int>(std::distance(begin, it));
}

int distance(const QString& srcPath, const QString& dstPath) {
    // calculate distance based on edit distance metric
    int n = srcPath.length();
    int m = dstPath.length();
    QVector<QVector<int>> dp(n + 1, QVector<int>(m + 1));
    for (int i = 0; i <= n; ++i) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= m; ++j) {
        dp[0][j] = j;
    }
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1,
                                 dp[i - 1][j - 1] + (srcPath[i - 1] == dstPath[j - 1] ? 0 : 1)});
        }
    }
    return dp[n][m];
}

void CMakeCbpParser::sortFiles()
{
    QList<QString> fileNames;
    for (const auto& fileNode : srcFileList) {
        fileNames.append(fileNode->getfilePath());
    }
    std::sort(fileNames.begin(), fileNames.end());

    int fallbackIndex = 0;
    int bestIncludeCount = -1;
    for (int i = 0; i < buildTargets.size(); ++i) {
        const auto &target = buildTargets.at(i);
        if (target.includeFiles.isEmpty()) {
            continue;
        }
        if (target.sourceDirectory == sourceDirectory && target.includeFiles.count() > bestIncludeCount) {
            bestIncludeCount = target.includeFiles.count();
            fallbackIndex = i;
        }
    }

    QString parentDirectory;
    CMakeBuildTarget *last = nullptr;
    for (const auto &fileName : fileNames) {
        const auto unitTargets = unitTargetMap[fileName];
        if (!unitTargets.isEmpty()) {
            for (const auto &unitTarget : unitTargets) {
                const auto index = indexOf(buildTargets, equal(&CMakeBuildTarget::title, unitTarget));
                if (index != -1) {
                    buildTargets[index].srcfiles.append(fileName);
                    continue;
                }
            }
            continue;
        }
        if (QFileInfo(fileName).dir().path() == parentDirectory && last) {
            last->srcfiles.append(fileName);
        } else {
            int bestDistance = std::numeric_limits<int>::max();
            int bestIndex = -1;
            int bestIncludeCount = -1;
            for (int i = 0; i < buildTargets.size(); ++i) {
                const auto &target = buildTargets.at(i);
                if (target.includeFiles.isEmpty()) {
                    continue;
                }
                const auto dist = distance(target.sourceDirectory, fileName);

                if (dist < bestDistance || (dist == bestDistance && target.includeFiles.count() > bestIncludeCount)) {
                    bestDistance = dist;
                    bestIncludeCount = target.includeFiles.count();
                    bestIndex = i;
                }
            }
            if (bestIndex == -1 && !buildTargets.isEmpty()) {
                bestIndex = fallbackIndex;
            }
            if (bestIndex != -1) {
                buildTargets[bestIndex].srcfiles.append(fileName);
                last = &buildTargets[bestIndex];
                parentDirectory = QFileInfo(fileName).dir().path();
            }
        }
    }
}

bool CMakeCbpParser::parseCbpFile(const QString &fileName,
                                  const QString &_sourceDirectory)
{
    buildDirectory = fileName;
    sourceDirectory = _sourceDirectory;

    QFile fi(fileName);
    if (fi.exists() && fi.open(QFile::ReadOnly)) {
        setDevice(&fi);

        while (!atEnd()) {
            readNext();
            if (name() == "CodeBlocks_project_file")
                parseCodeBlocks_project_file();
            else if (isStartElement())
                parseUnknownElement();
        }

        sortFiles();

        fi.close();

        return true;
    }
    return false;
}

void CMakeCbpParser::parseCodeBlocks_project_file()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (name() == "Project")
            parseProject();
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseProject()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (name() == "Option")
            parseOption();
        else if (name() == "Unit")
            parseUnit();
        else if (name() == "Build")
            parseBuild();
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseBuild()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (name() == "Target")
            parseBuildTarget();
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseBuildTarget()
{
    buildTarget.clear();

    if (attributes().hasAttribute("title"))
        buildTarget.title = attributes().value("title").toString();
    while (!atEnd()) {
        readNext();
        if (isEndElement()) {
            if (!buildTarget.title.endsWith("/fast")
                    && !buildTarget.title.endsWith("_automoc")) {
                if (buildTarget.output.isEmpty() && buildTarget.type == kExecutable)
                    buildTarget.type = kUtility;
                buildTargets.append(buildTarget);
            }
            return;
        } else if (name() == "Compiler") {
            parseCompiler();
        } else if (name() == "Option") {
            parseBuildTargetOption();
        } else if (name() == "MakeCommands") {
            parseMakeCommands();
        } else if (isStartElement()) {
            parseUnknownElement();
        }
    }
}

void CMakeCbpParser::parseBuildTargetOption()
{
    if (attributes().hasAttribute("output")) {
        buildTarget.output = (attributes().value("output").toString());
    } else if (attributes().hasAttribute("type")) {
        const QStringRef value = attributes().value("type");
        if (value == "0" || value == "1")
            buildTarget.type = kExecutable;
        else if (value == "2")
            buildTarget.type = kStaticLibrary;
        else if (value == "3")
            buildTarget.type = kDynamicLibrary;
        else
            buildTarget.type = kUtility;
    } else if (attributes().hasAttribute("working_dir")) {
        buildTarget.workingDirectory = attributes().value("working_dir").toString();

        QFile cmakeSourceInfoFile(buildTarget.workingDirectory
                                  + QStringLiteral("/CMakeFiles/CMakeDirectoryInformation.cmake"));
        if (cmakeSourceInfoFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&cmakeSourceInfoFile);
            const QLatin1String searchSource("SET(CMAKE_RELATIVE_PATH_TOP_SOURCE \"");
            while (!stream.atEnd()) {
                const QString lineTopSource = stream.readLine().trimmed();
                if (lineTopSource.startsWith(searchSource, Qt::CaseInsensitive)) {
                    QString src = lineTopSource.mid(searchSource.size());
                    src.chop(2);
                    buildTarget.sourceDirectory = src;
                    break;
                }
            }
        }

        if (buildTarget.sourceDirectory.isEmpty()) {
            buildTarget.sourceDirectory = sourceDirectory;
        }
    }
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (isStartElement())
            parseUnknownElement();
    }
}

QString CMakeCbpParser::getProjectName() const
{
    return projectName;
}

void CMakeCbpParser::parseOption()
{
    if (attributes().hasAttribute("title"))
        projectName = attributes().value("title").toString();

    if (attributes().hasAttribute("compiler"))
        compiler = attributes().value("compiler").toString();

    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseMakeCommands()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (name() == "Build")
            parseBuildTargetBuild();
        else if (name() == "Clean")
            parseBuildTargetClean();
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseBuildTargetBuild()
{
    if (attributes().hasAttribute("command"))
        buildTarget.makeCommand = (attributes().value("command").toString());
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseBuildTargetClean()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseCompiler()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (name() == "Add")
            parseAdd();
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseAdd()
{
    // CMake only supports <Add option=\> and <Add directory=\>
    const QXmlStreamAttributes addAttributes = attributes();

    QString includeDirectory = (addAttributes.value("directory").toString());

    // allow adding multiple times because order happens
    if (!includeDirectory.isEmpty())
        buildTarget.includeFiles.append(includeDirectory);

    QString compilerOption = addAttributes.value("option").toString();
    // defining multiple times a macro to the same value makes no sense
    if (!compilerOption.isEmpty() && !buildTarget.compilerOptions.contains(compilerOption)) {
        buildTarget.compilerOptions.append(compilerOption);
    }

    while (!atEnd()) {
        readNext();
        if (isEndElement())
            return;
        else if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseUnit()
{
    QString fileName = attributes().value("filename").toString();

    parsingCMakeUnit = false;
    unitTargets.clear();
    while (!atEnd()) {
        readNext();
        if (isEndElement()) {
            if (!fileName.endsWith(".rule") && !processedUnits.contains(fileName)) {
                // Now check whether we found a virtual element beneath
                if (parsingCMakeUnit) {
                    cmakeFileList.emplace_back(
                                std::make_unique<ProjectFile>(fileName, FileType::Project, false));
                } else {
                    bool generated = false;
                    QString onlyFileName = fileName;
                    if ((onlyFileName.startsWith("moc_") && onlyFileName.endsWith(".cxx"))
                            || (onlyFileName.startsWith("ui_") && onlyFileName.endsWith(".h"))
                            || (onlyFileName.startsWith("qrc_") && onlyFileName.endsWith(".cxx")))
                        generated = true;

                    if (fileName.endsWith(".qrc")) {
                        srcFileList.emplace_back(
                                    std::make_unique<ProjectFile>(fileName, FileType::Resource,
                                                                  generated));
                    } else {
                        srcFileList.emplace_back(
                                    std::make_unique<ProjectFile>(fileName, FileType::Source,
                                                                  generated));
                    }
                }
                unitTargetMap.insert(fileName, unitTargets);
                processedUnits.insert(fileName);
            }
            return;
        } else if (name() == "Option") {
            parseUnitOption();
        } else if (isStartElement()) {
            parseUnknownElement();
        }
    }
}

void CMakeCbpParser::parseUnitOption()
{
    const QXmlStreamAttributes optionAttributes = attributes();
    parsingCMakeUnit = optionAttributes.hasAttribute("virtualFolder");
    const QString target = optionAttributes.value("target").toString();
    if (!target.isEmpty())
        unitTargets.append(target);

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement())
            parseUnknownElement();
    }
}

void CMakeCbpParser::parseUnknownElement()
{
    Q_ASSERT(isStartElement());

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement())
            parseUnknownElement();
    }
}

bool CMakeCbpParser::hasCMakeFiles()
{
    return cmakeFileList.size() > 0;
}

const QList<CMakeBuildTarget> &CMakeCbpParser::getBuildTargets() const
{
    return buildTargets;
}

QString CMakeCbpParser::getCompilerName() const
{
    return compiler;
}
