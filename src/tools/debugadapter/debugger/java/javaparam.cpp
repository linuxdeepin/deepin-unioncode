/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer:
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
                                   const QString &triggerFile,
                                   const QString &jdkHome,
                                   const QString &debugJar)
{
    QString workspaceUrl = QUrl::fromLocalFile(workspace).toEncoded();
    QString triggerFileUrl = QUrl::fromLocalFile(triggerFile).toEncoded();
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
                            },
                            "triggerFiles": [
                                ")" + triggerFileUrl + R"("
                            ]
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

