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

#include <QString>

extern const QString T_NAV;
extern const QString T_MENU;
extern const QString T_FILEBROWSER;
extern const QString T_CODEEDITOR;
extern const QString T_DEBUGGER;
extern const QString T_WORKSPACE;

extern const QString D_RECENT_SHOW;
extern const QString D_RECENT_HIDE;
extern const QString D_EDIT_SHOW;
extern const QString D_EDIT_HIDE;
extern const QString D_DEBUG_SHOW;
extern const QString D_DEBUG_HIDE;
extern const QString D_RUNTIME_SHOW;
extern const QString D_RUNTIME_HIDE;

extern const QString D_WORKSPACE_GENERATE_BEGIN;
extern const QString D_WORKSPACE_GENERATE_END;
extern const QString D_DEBUG_EXECUTION_START;
extern const QString D_DEBUG_EXECUTION_END;
extern const QString D_DEBUG_EXECUTION_JUMP;
extern const QString D_DEBUG_EXECUTION_JUMP_CLEAN;
extern const QString D_FILE_OPENDOCUMENT;
extern const QString D_FILE_OPENFOLDER;
extern const QString D_ITEM_MENU_BUILD;
extern const QString D_ITEM_DOUBLECLICKED;

extern const QString D_MARGIN_DEBUG_POINT_ADD;
extern const QString D_MARGIN_DEBUG_POINT_REMOVE;
extern const QString D_MARGIN_DEBUG_POINT_CLEAN;

extern const QString P_FILELINE;       //value int
extern const QString P_FILEPATH;       //value QString
extern const QString P_PROJECTPATH;    //value QString
extern const QString P_TARGETPATH;     //value QString
extern const QString P_WORKSPACEFOLDER;//value QString
extern const QString P_BUILDSYSTEM;    //value QString
extern const QString P_BUILDDIRECTORY; //value QString
extern const QString P_BUILDFILEPATH;  //value QString
extern const QString P_BUILDARGUMENTS; //value QStringList
