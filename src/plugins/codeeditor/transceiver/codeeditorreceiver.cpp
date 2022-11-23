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
    if (D_SET_ANNOTATION == event.data()) {
        QVariant filePathVar = event.property(P_FILEPATH);
        QVariant fileLineVar = event.property(P_FILELINE);//.toInt(),
        QVariant textVar = event.property(P_TEXT);
        if (filePathVar.isValid() && fileLineVar.isValid() && textVar.isValid()) {
            int role = 0; // set default Note;
            /* Note = 767 Warning = 766 Error = 765 Fatal = 764*/
            role = event.property(P_ANNOTATION_ROLE).toInt();
            if (0 <= role && role <= 3) {
                role = 767 - role;
                DpfEventMiddleware::instance()->toSetAnnotation(
                            filePathVar.toString(),
                            fileLineVar.toInt(),
                            textVar.toString(),
                            role);
            }
        }
    } else if (D_CLEAN_ANNOTATION == event.data()) {
        QVariant filePathVar =  event.property(P_FILEPATH);
        if (filePathVar.isValid()) {
            DpfEventMiddleware::instance()->toCleanAnnotation(filePathVar.toString());
        }
    } else if (D_SET_LINE_BACKGROUND == event.data()) {

        QVariant colorVar = event.property(P_COLOR);
        QVariant filePathVar = event.property(P_FILEPATH);
        QVariant fileLineVar = event.property(P_FILELINE);
        if (colorVar.isValid() && filePathVar.isValid() && fileLineVar.isValid()) {
            DpfEventMiddleware::instance()->toSetLineBackground(
                        filePathVar.toString(),
                        fileLineVar.toInt(),
                        qvariant_cast<QColor>(colorVar));
        }

    } else if (D_DEL_LINE_BACKGROUND == event.data()) {

        QVariant filePathVar = event.property(P_FILEPATH);
        QVariant fileLineVar = event.property(P_FILELINE);
        if (filePathVar.isValid() && fileLineVar.isValid()) {
            DpfEventMiddleware::instance()->toDelLineBackground(
                        filePathVar.toString(),
                        fileLineVar.toInt());
        }

    } else if (D_CLEAN_LINE_BACKGROUND == event.data()) {

        QVariant filePathVar = event.property(P_FILEPATH);
        if (filePathVar.isValid()) {
            DpfEventMiddleware::instance()->toCleanLineBackground(
                        filePathVar.toString());
        }

    } else if (D_JUMP_CURSOR_CLEAN == event.data()) {

        return DpfEventMiddleware::instance()->toRunClean();

    } else if (D_OPENFILE == event.data()) {
        QVariant workspaceVar = event.property(P_WORKSPACEFOLDER);
        QVariant languageVar = event.property(P_LANGUAGE);
        QVariant filePathVar = event.property(P_FILEPATH);
        if (workspaceVar.isValid() && languageVar.isValid() && filePathVar.isValid()) {
            newlsp::ProjectKey proKey;
            proKey.language = languageVar.toString().toStdString();
            proKey.workspace = workspaceVar.toString().toStdString();
            return DpfEventMiddleware::instance()->toOpenFile(proKey, filePathVar.toString());
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
            return DpfEventMiddleware::instance()->toOpenFile(proKey, filePath);
        }
    } else if (D_SEARCH == event.data()) {

        QVariant srcTextVar = event.property(P_SRCTEXT);
        QVariant opeateTypeVar = event.property(P_OPRATETYPE);
        if (srcTextVar.isValid() && opeateTypeVar.isValid()) {
            return DpfEventMiddleware::instance()->toSearchText(
                        srcTextVar.toString(),
                        opeateTypeVar.toInt());
        }

    } else if (D_REPLACE == event.data()) {

        QVariant srcTextVar = event.property(P_SRCTEXT);
        QVariant destTextVar = event.property(P_DESTTEXT);
        QVariant opeateTypeVar = event.property(P_OPRATETYPE);
        if (srcTextVar.isValid() && destTextVar.isValid() && opeateTypeVar.isValid()) {
            return DpfEventMiddleware::instance()->toReplaceText(
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
            return DpfEventMiddleware::instance()->toJumpFileLine(
                        key,
                        filePathVar.toString(),
                        fileLineVar.toInt());
        } else if (filePathVar.isValid() && fileLineVar.isValid()) {
            return DpfEventMiddleware::instance()->toRunFileLine(
                        filePathVar.toString(),
                        fileLineVar.toInt());
        }
    } else if (event.data() == editor.openDocument.name) {
        QString language = event.property(editor.openDocument.pKeys[0]).toString();
        QString filePath = event.property(editor.openDocument.pKeys[1]).toString();
        newlsp::ProjectKey proKey;
        proKey.language = language.toStdString();
        return DpfEventMiddleware::instance()->toOpenFile(proKey, filePath);
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
            return DpfEventMiddleware::instance()->toJumpFileLine(key, filePath, fileLine.toInt());
        } else if (!filePath.isEmpty() && !fileLine.isEmpty()) {
            return DpfEventMiddleware::instance()->toRunFileLine(filePath, fileLine.toInt());
        }
    } else if (event.data() == actionanalyse.analyseDone.name) {
        QString workspace = event.property(actionanalyse.analyseDone.pKeys[0]).toString();
        QString language = event.property(actionanalyse.analyseDone.pKeys[1]).toString();
        QString storage = event.property(actionanalyse.analyseDone.pKeys[2]).toString();
        QVariant analyseDataVar = event.property(actionanalyse.analyseDone.pKeys[0]);
        if (analyseDataVar.canConvert<AnalysedData>()) {
           // to do Huang Yu, do lsp token and data token
        }
    }
}

DpfEventMiddleware *DpfEventMiddleware::instance()
{
    static DpfEventMiddleware ins;
    return &ins;
}
