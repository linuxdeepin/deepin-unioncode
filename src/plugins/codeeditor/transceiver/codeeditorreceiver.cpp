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
    return {T_CODEEDITOR, editor.topic, actionanalyse.topic};
}

void CodeEditorReceiver::eventProcess(const dpf::Event &event)
{
    if (D_SET_LINE_BACKGROUND == event.data()) {

        QVariant colorVar = event.property(P_COLOR);
        QVariant filePathVar = event.property(P_FILEPATH);
        QVariant fileLineVar = event.property(P_FILELINE);
        if (colorVar.isValid() && filePathVar.isValid() && fileLineVar.isValid()) {
            EditorCallProxy::instance()->toSetLineBackground(
                        filePathVar.toString(),
                        fileLineVar.toInt(),
                        qvariant_cast<QColor>(colorVar));
        }

    } else if (D_DEL_LINE_BACKGROUND == event.data()) {

        QVariant filePathVar = event.property(P_FILEPATH);
        QVariant fileLineVar = event.property(P_FILELINE);
        if (filePathVar.isValid() && fileLineVar.isValid()) {
            EditorCallProxy::instance()->toDelLineBackground(
                        filePathVar.toString(),
                        fileLineVar.toInt());
        }

    } else if (D_CLEAN_LINE_BACKGROUND == event.data()) {

        QVariant filePathVar = event.property(P_FILEPATH);
        if (filePathVar.isValid()) {
            EditorCallProxy::instance()->toCleanLineBackground(
                        filePathVar.toString());
        }

    } else if (D_JUMP_CURSOR_CLEAN == event.data()) {

        return EditorCallProxy::instance()->toRunClean();

    } else if (D_OPENFILE == event.data()) {
        QVariant workspaceVar = event.property(P_WORKSPACEFOLDER);
        QVariant languageVar = event.property(P_LANGUAGE);
        QVariant filePathVar = event.property(P_FILEPATH);
        if (workspaceVar.isValid() && languageVar.isValid() && filePathVar.isValid()) {
            newlsp::ProjectKey proKey;
            proKey.language = languageVar.toString().toStdString();
            proKey.workspace = workspaceVar.toString().toStdString();
            return EditorCallProxy::instance()->toOpenFile(proKey, filePathVar.toString());
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
            return EditorCallProxy::instance()->toOpenFile(proKey, filePath);
        }
    } else if (D_SEARCH == event.data()) {

        QVariant srcTextVar = event.property(P_SRCTEXT);
        QVariant opeateTypeVar = event.property(P_OPRATETYPE);
        if (srcTextVar.isValid() && opeateTypeVar.isValid()) {
            return EditorCallProxy::instance()->toSearchText(
                        srcTextVar.toString(),
                        opeateTypeVar.toInt());
        }

    } else if (D_REPLACE == event.data()) {

        QVariant srcTextVar = event.property(P_SRCTEXT);
        QVariant destTextVar = event.property(P_DESTTEXT);
        QVariant opeateTypeVar = event.property(P_OPRATETYPE);
        if (srcTextVar.isValid() && destTextVar.isValid() && opeateTypeVar.isValid()) {
            return EditorCallProxy::instance()->toReplaceText(
                        srcTextVar.toString(),
                        destTextVar.toString(),
                        opeateTypeVar.toInt());
        }

    } else if (D_JUMP_TO_LINE == event.data()) {
        QVariant wpFolderVar = event.property(P_WORKSPACEFOLDER);
        QVariant languageVar = event.property(P_LANGUAGE);
        QVariant filePathVar = event.property(P_FILEPATH);
        QVariant fileLineVar = event.property(P_FILELINE);
        if (wpFolderVar.isValid() && languageVar.isValid()
                && filePathVar.isValid() && fileLineVar.isValid()) {
            newlsp::ProjectKey key;
            key.language = wpFolderVar.toString().toStdString();
            key.workspace = languageVar.toString().toStdString();
            return EditorCallProxy::instance()->toJumpFileLine(
                        key,
                        filePathVar.toString(),
                        fileLineVar.toInt());
        } else if (filePathVar.isValid() && fileLineVar.isValid()) {
            return EditorCallProxy::instance()->toRunFileLine(
                        filePathVar.toString(),
                        fileLineVar.toInt());
        }
    } else if (event.data() == editor.openDocument.name) {
        QString language = event.property(editor.openDocument.pKeys[0]).toString();
        QString filePath = event.property(editor.openDocument.pKeys[1]).toString();
        newlsp::ProjectKey proKey;
        proKey.language = language.toStdString();
        return EditorCallProxy::instance()->toOpenFile(proKey, filePath);
    } else if (editor.jumpToLine.name == event.data()) {
        QString workspacePKey = editor.jumpToLine.pKeys[0];
        QString languagePKey = editor.jumpToLine.pKeys[1];
        QString filePathPKey = editor.jumpToLine.pKeys[2];
        QString fileLinePKey = editor.jumpToLine.pKeys[3];
        QString workspace = event.property(workspacePKey).toString();
        QString language = event.property(languagePKey).toString();
        QString filePath = event.property(filePathPKey).toString();
        QString fileLine = event.property(fileLinePKey).toString();
        if (!workspace.isEmpty() && !language.isEmpty()
                && !filePath.isEmpty() && !fileLine.isEmpty()) {
            newlsp::ProjectKey key;
            key.language = language.toStdString();
            key.workspace = workspace.toStdString();
            return EditorCallProxy::instance()->toJumpFileLine(key, filePath, fileLine.toInt());
        } else if (!filePath.isEmpty() && !fileLine.isEmpty()) {
            return EditorCallProxy::instance()->toRunFileLine(filePath, fileLine.toInt());
        }
    } else if (event.data() == actionanalyse.analyseDone.name) {
        QString workspace = event.property(actionanalyse.analyseDone.pKeys[0]).toString();
        QString language = event.property(actionanalyse.analyseDone.pKeys[1]).toString();
        QString storage = event.property(actionanalyse.analyseDone.pKeys[2]).toString();
        QVariant analyseDataVar = event.property(actionanalyse.analyseDone.pKeys[0]);
        if (analyseDataVar.canConvert<AnalysedData>()) {
            auto analyseData = qvariant_cast<AnalysedData>(analyseDataVar);
            EditorCallProxy::instance()->toSetAnalysedData(analyseData);
        }
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
    }
}

EditorCallProxy *EditorCallProxy::instance()
{
    static EditorCallProxy ins;
    return &ins;
}
