/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "codeeditorreceiver.h"

#include "common/common.h"
#include "services/project/projectservice.h"

#include "mainframe/texteditkeeper.h"

CodeEditorReceiver::CodeEditorReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<CodeEditorReceiver> ()
{

}

dpf::EventHandler::Type CodeEditorReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList CodeEditorReceiver::topics()
{
    return {editor.topic, actionanalyse.topic, project.topic};
}

void CodeEditorReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.activedProject.name) {
        QVariant proInfoVar = event.property(project.activedProject.pKeys[0]);
        TextEditKeeper::saveProjectInfo(qvariant_cast<dpfservice::ProjectInfo>(proInfoVar));
    } else if (event.data() == project.deletedProject.name) {
        QVariant proInfoVar = event.property(project.deletedProject.pKeys[0]);
        TextEditKeeper::removeProjectInfo(qvariant_cast<dpfservice::ProjectInfo>(proInfoVar));
    } else if (event.data() == project.createdProject.name) {
        QVariant proInfoVar = event.property(project.createdProject.pKeys[0]);
        TextEditKeeper::saveProjectInfo(qvariant_cast<dpfservice::ProjectInfo>(proInfoVar));
    }else if (event.data() == editor.cleanRunning.name) {
        return EditorCallProxy::instance()->toRunClean();
    } else if (D_OPENFILE == event.data()) {
        QVariant workspaceVar = event.property(P_WORKSPACEFOLDER);
        QVariant languageVar = event.property(P_LANGUAGE);
        QVariant filePathVar = event.property(P_FILEPATH);
        if (workspaceVar.isValid() && languageVar.isValid() && filePathVar.isValid()) {
            newlsp::ProjectKey proKey;
            proKey.language = languageVar.toString().toStdString();
            proKey.workspace = workspaceVar.toString().toStdString();
            return EditorCallProxy::instance()->toOpenFileWithKey(proKey, filePathVar.toString());
        }
    } else if (editor.openFile.name == event.data()) {
        QString workspacePKey = editor.openFile.pKeys[0];
        QString languagePKey = editor.openFile.pKeys[1];
        QString filePathPKey = editor.openFile.pKeys[2];
        QString workspace = event.property(workspacePKey).toString();
        QString language = event.property(languagePKey).toString();
        QString filePath = event.property(filePath).toString();
        if (!workspace.isEmpty() && !language.isEmpty() && !language.isEmpty()) {
            newlsp::ProjectKey proKey;
            proKey.language = language.toStdString();
            proKey.workspace = workspace.toStdString();
            return EditorCallProxy::instance()->toOpenFileWithKey(proKey, filePath);
        }
    } else if (event.data() == editor.searchText.name) {
        QString srcText = event.property(editor.searchText.pKeys[0]).toString();
        int findType = event.property(editor.searchText.pKeys[1]).toInt();
        return EditorCallProxy::instance()->toSearchText(srcText, findType);
    } else if (event.data() == editor.replaceText.name) {
        QString srcText = event.property(editor.replaceText.pKeys[0]).toString();
        QString targetText = event.property(editor.replaceText.pKeys[1]).toString();
        int replaceType = event.property(editor.replaceText.pKeys[2]).toInt();
        return EditorCallProxy::instance()->toReplaceText(srcText, targetText, replaceType);
    }  else if (event.data() == editor.openDocument.name) {
        QString language = event.property(editor.openDocument.pKeys[0]).toString();
        QString filePath = event.property(editor.openDocument.pKeys[1]).toString();
        newlsp::ProjectKey proKey{};
        proKey.language = language.toStdString();
        return EditorCallProxy::instance()->toOpenFileWithKey(proKey, filePath);
    }  else if (event.data() == editor.jumpToLine.name) {
        QString workspace = TextEditKeeper::projectInfo().workspaceFolder();
        if (workspace.isEmpty()) {
            workspace = QDir::homePath();
        }
        QString filePath = event.property(editor.jumpToLine.pKeys[0]).toString();
        QString language = support_file::Language::id(filePath);
        newlsp::ProjectKey proKey;
        proKey.language = language.toStdString();
        proKey.workspace = workspace.toStdString();
        int line = event.property(editor.jumpToLine.pKeys[1]).toInt();
        return EditorCallProxy::instance()->toJumpFileLineWithKey(proKey, filePath, line);
    } else if (event.data() == actionanalyse.analyseDone.name) {
        QString workspace = event.property(actionanalyse.analyseDone.pKeys[0]).toString();
        QString language = event.property(actionanalyse.analyseDone.pKeys[1]).toString();
        QString storage = event.property(actionanalyse.analyseDone.pKeys[2]).toString();
        AnalysedData analyseData = qvariant_cast<AnalysedData>(event.property(actionanalyse.analyseDone.pKeys[3]));
        TextEditKeeper::setAnalysedData(analyseData);
    } else if (event.data() == editor.setAnnotation.name) {
        QString filePath = event.property(editor.setAnnotation.pKeys[0]).toString();
        int line = event.property(editor.setAnnotation.pKeys[1]).toInt();
        QString title = event.property(editor.setAnnotation.pKeys[2]).toString();
        AnnotationInfo annInfo = qvariant_cast<AnnotationInfo>(event.property(editor.setAnnotation.pKeys[3]));
        EditorCallProxy::instance()->toSetAnnotation(filePath, line, title, annInfo);
    } else if (event.data() == editor.cleanAnnotation.name) {
        QString filePath = event.property(editor.cleanAnnotation.pKeys[0]).toString();
        QString title = event.property(editor.cleanAnnotation.pKeys[1]).toString();
        EditorCallProxy::instance()->toCleanAnnotation(filePath, title);
    } else if (event.data() == editor.setLineBackground.name) {
        QString filePath = event.property(editor.setLineBackground.pKeys[0]).toString();
        int line = event.property(editor.setLineBackground.pKeys[1]).toInt();
        QColor color = qvariant_cast<QColor>(event.property(editor.setLineBackground.pKeys[2]));
        EditorCallProxy::instance()->toSetLineBackground(filePath, line, color);
    } else if (event.data() == editor.delLineBackground.name) {
        QString filePath = event.property(editor.delLineBackground.pKeys[0]).toString();
        int line = event.property(editor.delLineBackground.pKeys[1]).toInt();
        EditorCallProxy::instance()->toDelLineBackground(filePath, line);
    } else if (event.data() == editor.cleanLineBackground.name) {
        QString filePath = event.property(editor.cleanLineBackground.pKeys[0]).toString();
        EditorCallProxy::instance()->toCleanLineBackground(filePath);
    } else if (event.data() == editor.runningToLine.name) {
        auto proInfo = TextEditKeeper::projectInfo();
        QString workspace = proInfo.workspaceFolder();
        if (workspace.isEmpty()) {
            workspace = QDir::homePath();
        }
        QString language = proInfo.language();
        QString filePath = event.property(editor.runningToLine.pKeys[0]).toString();
        if (language.isEmpty()) {
            language = support_file::Language::id(filePath);
        }
        newlsp::ProjectKey proKey;
        proKey.language = language.toStdString();
        proKey.workspace = workspace.toStdString();
        int line = event.property(editor.runningToLine.pKeys[1]).toInt();
        EditorCallProxy::instance()->toRunFileLineWithKey(proKey, filePath, line);
    } else if (event.data() == editor.jumpToLineWithKey.name){
        QString workspace = event.property(editor.jumpToLineWithKey.pKeys[0]).toString();
        QString language = event.property(editor.jumpToLineWithKey.pKeys[1]).toString();
        newlsp::ProjectKey proKey;
        proKey.language = language.toStdString();
        proKey.workspace = workspace.toStdString();
        QString filePath = event.property(editor.jumpToLineWithKey.pKeys[2]).toString();
        int line = event.property(editor.jumpToLineWithKey.pKeys[3]).toInt();
        EditorCallProxy::instance()->toJumpFileLineWithKey(proKey, filePath, line);
    } else if (event.data() == editor.openFileWithKey.name) {
        QString workspace = event.property(editor.openFileWithKey.pKeys[0]).toString();
        QString language = event.property(editor.openFileWithKey.pKeys[1]).toString();
        newlsp::ProjectKey proKey;
        proKey.language = language.toStdString();
        proKey.workspace = workspace.toStdString();
        QString filePath = event.property(editor.openFileWithKey.pKeys[2]).toString();
        EditorCallProxy::instance()->toOpenFileWithKey(proKey, filePath);
    }
}

EditorCallProxy::EditorCallProxy()
{

}

EditorCallProxy *EditorCallProxy::instance()
{
    static EditorCallProxy proxy;
    return &proxy;
}
