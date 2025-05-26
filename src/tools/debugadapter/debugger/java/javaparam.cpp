// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "javaparam.h"

#include <QFileInfo>
#include <QUrl>

JavaParam::JavaParam(QObject *parent)
    : QObject(parent)
{

}

JavaParam::~JavaParam()
{

}

QString JavaParam::getInitBackendParam(const QString &port,
                                       const QString &javaPath,
                                       const QString &launcherPath,
                                       const QString &heapDumpPath,
                                       const QString &configLinuxPath,
                                       const QString &dataPath)
{
    QString param = javaPath + " " +
                    "-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=" + port + ",quiet=y "
                    "--add-modules=ALL-SYSTEM "
                    "--add-opens java.base/java.util=ALL-UNNAMED "
                    "--add-opens java.base/java.lang=ALL-UNNAMED "
                    "--add-opens java.base/sun.nio.fs=ALL-UNNAMED "
                    "-Declipse.application=org.eclipse.jdt.ls.core.id1 "
                    "-Dosgi.bundles.defaultStartLevel=4 "
                    "-Declipse.product=org.eclipse.jdt.ls.core.product "
                    "-Dlog.level=ALL "
                    "-Djava.import.generatesMetadataFilesAtProjectRoot=false "
                    "-Dfile.encoding=utf8 "
                    "-XX:+UseParallelGC "
                    "-XX:GCTimeRatio=4 "
                    "-XX:AdaptiveSizePolicyWeight=90 "
                    "-Dsun.zip.disableMemoryMapping=true "
                    "-Xmx1G -Xms100m -Xlog:jni+resolve=off "
                    "-XX:+HeapDumpOnOutOfMemoryError "
                    "-XX:HeapDumpPath=" + heapDumpPath + " "
                    "-jar " + launcherPath + " "
                    "-configuration " + configLinuxPath + " "
                    "-data " + dataPath;
    return param;
}

QString JavaParam::getLSPInitParam(const int requestId,
                                   const int pid,
                                   const QString &workspace,
                                   const QString &jdkHome,
                                   const QString &debugJar)
{
    QString workspaceUrl = QUrl::fromLocalFile(workspace).toEncoded();
    QString param = R"({
                    "jsonrpc": "2.0",
                    "id": )" + QString::number(requestId) + R"(,
                    "method": "initialize",
                    "params": {
                        "processId": )" + QString::number(pid) + R"(,
                        "clientInfo": {
                            "name": "Union Code",
                            "version": "1.0.0"
                        },
                        "locale": "zh-cn",
                        "rootPath": ")" + workspaceUrl + R"(",
                        "rootUri": ")" + workspaceUrl + R"(",
                        "capabilities": {

                        },
                        "initializationOptions": {
                            "bundles": [
                                ")" + debugJar + R"("
                            ],
                            "workspaceFolders": [
                                ")" + workspaceUrl + R"("
                            ],
                            "settings": {
                                "java": {
                                    "home": ")" + jdkHome + R"(",
                                    "sources": {
                                        "organizeImports": {
                                            "starThreshold": 99,
                                            "staticStarThreshold": 99
                                        }
                                    },
                                    "import": {
                                         "maven": {
                                             "enabled": true,
                                             "offline": {
                                                 "enabled": false
                                             }
                                         },
                                         "gradle": {
                                             "enabled": true,
                                             "wrapper": {
                                                 "enabled": true
                                             },
                                             "version": null,
                                             "home": null,
                                             "java": {
                                                 "home": null
                                             },
                                             "offline": {
                                                 "enabled": false
                                             },
                                             "arguments": null,
                                             "jvmArguments": null,
                                             "user": {
                                                 "home": null
                                             }
                                         },
                                         "exclusions": [
                                             "**/node_modules/**",
                                             "**/.metadata/**",
                                             "**/archetype-resources/**",
                                             "**/META-INF/maven/**"
                                         ],
                                         "generatesMetadataFilesAtProjectRoot": false
                                     },
                                    "debug": {
                                        "logLevel": "verbose",
                                        "settings": {
                                            "showHex": false,
                                            "showStaticVariables": false,
                                            "showQualifiedNames": false,
                                            "showLogicalStructure": true,
                                            "showToString": true,
                                            "maxStringLength": 0,
                                            "numericPrecision": 0,
                                            "hotCodeReplace": "manual",
                                            "enableRunDebugCodeLens": true,
                                            "forceBuildBeforeLaunch": true,
                                            "onBuildFailureProceed": false,
                                            "console": "integratedTerminal",
                                            "exceptionBreakpoint": {
                                                "skipClasses": []
                                            },
                                            "stepping": {
                                                "skipClasses": [],
                                                "skipSynthetics": false,
                                                "skipStaticInitializers": false,
                                                "skipConstructors": false
                                            },
                                            "jdwp": {
                                                "limitOfVariablesPerJdwpRequest": 100,
                                                "requestTimeout": 3000,
                                                "async": "auto"
                                            },
                                            "vmArgs": ""
                                        }
                                    },
                                    "silentNotification": false
                                }
                            },
                            "extendedClientCapabilities": {
                                "progressReportProvider": true,
                                "classFileContentsSupport": true,
                                "overrideMethodsPromptSupport": true,
                                "hashCodeEqualsPromptSupport": true,
                                "advancedOrganizeImportsSupport": true,
                                "generateToStringPromptSupport": true,
                                "advancedGenerateAccessorsSupport": true,
                                "generateConstructorsPromptSupport": true,
                                "generateDelegateMethodsPromptSupport": true,
                                "advancedExtractRefactoringSupport": true,
                                "inferSelectionSupport": [
                                    "extractMethod",
                                    "extractVariable",
                                    "extractField"
                                ],
                                "moveRefactoringSupport": true,
                                "clientHoverProvider": true,
                                "clientDocumentSymbolProvider": true,
                                "gradleChecksumWrapperPromptSupport": true,
                                "resolveAdditionalTextEditsSupport": true,
                                "advancedIntroduceParameterRefactoringSupport": true,
                                "actionableRuntimeNotificationSupport": true,
                                "shouldLanguageServerExitOnShutdown": true,
                                "onCompletionItemSelectedCommand": "editor.action.triggerParameterHints"
                            }
                        },
                        "trace": "verbose",
                        "workspaceFolders": [{
                                    "uri": ")" + workspaceUrl + R"(",
                                    "name": ")" + QFileInfo(workspace).fileName() + R"("
                                }]
                    }
                })";

    return param;
}

QString JavaParam::getLSPInitilizedParam(const int requestId)
{
    QString param = R"({
                    "jsonrpc": "2.0",
                    "id": )" + QString::number(requestId) + R"(,
                    "method": "initialized",
                    "params": {}
                })";

    return param;
}

QString JavaParam::getLaunchJavaDAPParam(const int requestId)
{
    QString param = R"({
                    "jsonrpc": "2.0",
                    "id": )" + QString::number(requestId) + R"(,
                    "method": "workspace/executeCommand",
                    "params": {
                        "command": "vscode.java.startDebugSession",
                        "arguments": []
                    }
                })";

    return param;
}


QString JavaParam::getResolveMainClassParam(const int requestId,
                                            const QString &workspace)
{
    QString workspaceUrl = QUrl::fromLocalFile(workspace).toEncoded();
    QString param = R"({
                    "jsonrpc": "2.0",
                    "id": )" + QString::number(requestId) + R"(,
                    "method": "workspace/executeCommand",
                    "params": {
                        "command": "vscode.java.resolveMainClass",
                        "arguments": [")" + workspaceUrl + R"("]
                    }
                })";

    return param;
}

QString JavaParam::getResolveClassPathParam(const int requestId,
                                            const QString &mainClass,
                                            const QString &projectName)
{
    QString param = R"({
                    "jsonrpc": "2.0",
                    "id": )" + QString::number(requestId) + R"(,
                    "method": "workspace/executeCommand",
                    "params": {
                        "command": "vscode.java.resolveClasspath",
                        "arguments": [")" + mainClass + R"(", ")" + projectName + R"("]
                    }
                })";

    return param;
}
