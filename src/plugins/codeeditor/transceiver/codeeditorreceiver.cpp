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
    return {T_CODEEDITOR}; //绑定menu 事件
}

void CodeEditorReceiver::eventProcess(const dpf::Event &event)
{
    if (!topics().contains(event.topic()))
        abort();

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
            lsp::Head head { workspaceVar.toString(), languageVar.toString() };
            return DpfEventMiddleware::instance()->toOpenFile(head, filePathVar.toString());
        }

    } else if (D_OPENDOCUMENT == event.data()) {

        QVariant filePathVar = event.property(P_FILELINE);
        if (filePathVar.isValid()) {
            return DpfEventMiddleware::instance()->toOpenFile(
                        filePathVar.toString());
        }

    } else if (D_OPENPROJECT == event.data()) {

        qInfo() << event;
        return;

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
            return DpfEventMiddleware::instance()->toJumpFileLine(
                        lsp::Head(wpFolderVar.toString(), languageVar.toString()),
                        filePathVar.toString(),
                        fileLineVar.toInt());
        } else if (filePathVar.isValid() && fileLineVar.isValid()) {
            return DpfEventMiddleware::instance()->toRunFileLine(
                        filePathVar.toString(),
                        fileLineVar.toInt());
        }

    }
}

DpfEventMiddleware *DpfEventMiddleware::instance()
{
    static DpfEventMiddleware ins;
    return &ins;
}
