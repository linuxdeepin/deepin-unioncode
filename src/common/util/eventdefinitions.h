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
#ifndef EVENTDEFINITIONS_H
#define EVENTDEFINITIONS_H

#endif // EVENTDEFINITIONS_H

#include "framework.h"

#include <QString>

OPI_OBJECT(recent,
           OPI_INTERFACE(saveOpenedProject, "filePath", "kitName", "language", "workspace")
           )

OPI_OBJECT(project,
           OPI_INTERFACE(openProject,  "filePath", "kitName", "language", "workspace")
           OPI_INTERFACE(activedProject, "projectInfo")
           OPI_INTERFACE(deletedProject, "projectInfo")
           OPI_INTERFACE(createdProject, "projectInfo")
           )

OPI_OBJECT(debugger,
           OPI_INTERFACE(prepareDebugProgress, "message")
           OPI_INTERFACE(prepareDebugDone, "succeed", "message")
           )

OPI_OBJECT(editor,
           OPI_INTERFACE(openFile, "workspace", "language", "filePath")
           OPI_INTERFACE(jumpToLine, "workspace", "language", "filePath", "line")
           OPI_INTERFACE(openDocument, "language", "filePath")
           )

OPI_OBJECT(symbol,
           OPI_INTERFACE(parse, "workspace", "language", "storage")
           OPI_INTERFACE(parseDone, "workspace", "language", "storage")
           )

OPI_OBJECT(navigation,
           OPI_INTERFACE(doSwitch, "actionText")
           )

extern const QString T_NAV;
extern const QString T_MENU;
extern const QString T_FILEBROWSER;
extern const QString T_CODEEDITOR;
extern const QString T_DEBUGGER;
extern const QString T_BUILDER;
extern const QString T_WORKSPACE;
extern const QString T_RECENT;
extern const QString T_PROJECT;
extern const QString T_SYMBOL;
extern const QString T_PROCESSMESSAGE;
extern const QString T_FIND;
extern const QString T_COLLABORATORS;

extern const QString D_ACTION_SWITCH;

extern const QString D_WORKSPACE_GENERATE_BEGIN;
extern const QString D_WORKSPACE_GENERATE_END;
extern const QString D_DEBUG_EXECUTION_START;
extern const QString D_DEBUG_EXECUTION_END;
extern const QString D_DEBUG_EXECUTION_JUMP;
extern const QString D_JUMP_CURSOR_CLEAN;
extern const QString D_BUILD_STATE;
extern const QString D_OPENDOCUMENT;
extern const QString D_OPENPROJECT;
extern const QString D_ITEM_MENU_BUILD;
extern const QString D_BUILD_TARGET;
extern const QString D_SHOW;
extern const QString D_HIDE;
extern const QString D_ADDTEXT;
extern const QString D_CRETED;
extern const QString D_DELETED;
extern const QString D_ACTIVED;
extern const QString D_JUMP_TO_LINE;
extern const QString D_SET_ANNOTATION;
extern const QString D_CLEAN_ANNOTATION;
extern const QString D_SET_LINE_BACKGROUND;
extern const QString D_DEL_LINE_BACKGROUND;
extern const QString D_CLEAN_LINE_BACKGROUND;
extern const QString D_BUILD_COMMAND;
extern const QString D_MENU_BUILD;
extern const QString D_MENU_REBUILD;
extern const QString D_MENU_CLEAN;
extern const QString D_OPEN_REPOS;

extern const QString D_MARGIN_DEBUG_POINT_ADD;
extern const QString D_MARGIN_DEBUG_POINT_REMOVE;
extern const QString D_MARGIN_DEBUG_POINT_CLEAN;

extern const QString D_SEARCH;
extern const QString D_REPLACE;
extern const QString D_OPENFILE;

extern const QString P_ACTION_TEXT;    // value QString
extern const QString P_FILELINE;       // value int
extern const QString P_FILEPATH;       // value QString
extern const QString P_PROJECTPATH;    // value QString
extern const QString P_TARGETPATH;     // value QString
extern const QString P_WORKSPACEFOLDER;// value QString
extern const QString P_COMPILEFOLDER;  // value QString
extern const QString P_BUILDSYSTEM;    // value QString
extern const QString P_BUILDDIRECTORY; // value QString
extern const QString P_BUILDFILEPATH;  // value QString
extern const QString P_BUILDARGUMENTS; // value QStringList
extern const QString P_TEXT;           // value QString
extern const QString P_ORIGINCMD;       // value QString, event sender id.
extern const QString P_PERCENT;        // value int
extern const QString P_MAX_PERCENT;    // value int
extern const QString P_STATE;          // value int
extern const QString P_KITNAME;        // value QString
extern const QString P_LANGUAGE;       // value QString
extern const QString P_PROJECT_INFO;   // value dpfService::ProjectInfo
extern const QString P_ANNOTATION_ROLE; // only with 0:Note 1:Warning 2:Error 3:Fatal
extern const QString P_COLOR;          // value QColor

extern const QString P_OPRATETYPE;
extern const QString P_SRCTEXT;
extern const QString P_DESTTEXT;

extern const QString P_BUILDPROGRAM;
extern const QString P_BUILDWORKINGDIR;
