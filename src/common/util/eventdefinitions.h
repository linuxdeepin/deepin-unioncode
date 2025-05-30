// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTDEFINITIONS_H
#define EVENTDEFINITIONS_H

#include "common/lsp/protocol/newprotocol.h"
#include "framework/framework.h"

#include <QString>

OPI_OBJECT(recent,
           OPI_INTERFACE(saveOpenedProject, "kitName", "language", "workspace")
           OPI_INTERFACE(saveOpenedFile, "filePath")
           )

OPI_OBJECT(project,
           // in
           OPI_INTERFACE(openProject, "kitName", "language", "workspace")
           OPI_INTERFACE(openProjectByPath, "directory")
           OPI_INTERFACE(activeProject, "kitName", "language", "workspace")
           // out
           OPI_INTERFACE(activatedProject, "projectInfo")
           OPI_INTERFACE(deletedProject, "projectInfo")
           OPI_INTERFACE(createdProject, "projectInfo")
           OPI_INTERFACE(projectUpdated, "projectInfo")
           OPI_INTERFACE(projectNodeExpanded, "modelIndex")
           OPI_INTERFACE(projectNodeCollapsed, "modelIndex")
           OPI_INTERFACE(fileDeleted, "filePath", "kit")

           OPI_INTERFACE(openProjectPropertys, "projectInfo")
           )

OPI_OBJECT(debugger,
           OPI_INTERFACE(prepareDebugProgress, "message")
           OPI_INTERFACE(prepareDebugDone, "succeed", "message")
           OPI_INTERFACE(executeStart)
           OPI_INTERFACE(enableBreakpoints, "breakpoints")
           OPI_INTERFACE(disableBreakpoints, "breakpoints")
           )

OPI_OBJECT(editor,
           // in
           OPI_INTERFACE(openFile, "workspace", "fileName")
           OPI_INTERFACE(closeFile, "fileName")
           OPI_INTERFACE(back)
           OPI_INTERFACE(forward)
           OPI_INTERFACE(gotoLine, "fileName", "line")
           OPI_INTERFACE(gotoPosition, "fileName", "line", "column")

           OPI_INTERFACE(setDebugLine, "fileName", "line")
           OPI_INTERFACE(removeDebugLine)
           OPI_INTERFACE(setModifiedAutoReload, "fileName", "flag")
           OPI_INTERFACE(addBreakpoint, "fileName", "line", "enabled")
           OPI_INTERFACE(removeBreakpoint, "fileName", "line")
           OPI_INTERFACE(setBreakpointEnabled, "fileName", "line", "enabled")
           OPI_INTERFACE(clearAllBreakpoint)
           // out
           OPI_INTERFACE(lineChanged, "fileName", "startLine", "added")
           OPI_INTERFACE(fileOpened, "fileName")
           OPI_INTERFACE(fileClosed, "fileName")
           OPI_INTERFACE(fileSaved, "fileName")
           OPI_INTERFACE(switchedFile, "fileName")
           OPI_INTERFACE(breakpointAdded, "fileName", "line", "enabled")
           OPI_INTERFACE(breakpointRemoved, "fileName", "line")
           OPI_INTERFACE(breakpointStatusChanged, "fileName", "line", "enabled")
           OPI_INTERFACE(textChanged)
           OPI_INTERFACE(cursorPositionChanged, "fileName", "line", "index")
           OPI_INTERFACE(selectionChanged, "fileName", "lineFrom", "indexFrom", "lineTo", "indexTo")
           OPI_INTERFACE(inlineWidgetClosed)

           //right-cliked menu,  Related to debugging
           OPI_INTERFACE(setBreakpointCondition, "fileName", "line")
           OPI_INTERFACE(jumpToLine, "fileName", "line")
           OPI_INTERFACE(runToLine, "fileName", "line")
           OPI_INTERFACE(contextMenu, "menu")
           OPI_INTERFACE(marginMenu, "menu")
           )

OPI_OBJECT(symbol,
           OPI_INTERFACE(parse, "workspace", "language", "storage")
           OPI_INTERFACE(parseDone, "workspace", "language", "storage", "success")
           )

OPI_OBJECT(uiController,
           //receivce
           OPI_INTERFACE(doSwitch, "actionText")
           OPI_INTERFACE(switchContext, "name")
           OPI_INTERFACE(switchWorkspace, "name")

           //send
           OPI_INTERFACE(switchToWidget, "name")
           OPI_INTERFACE(modeRaised, "mode")
           )

OPI_OBJECT(notifyManager,
           // out
           OPI_INTERFACE(actionInvoked, "actionId")
           )

OPI_OBJECT(actionanalyse,
           // in
           OPI_INTERFACE(analyse, "workspace", "language", "storage")
           // analysedData value custom struct from AnalysedData
           OPI_INTERFACE(analyseDone, "workspace", "language", "storage", "analysedData")
           // out
           OPI_INTERFACE(enabled, "flag")
           )

OPI_OBJECT(commandLine,
           OPI_INTERFACE(build)
           )

OPI_OBJECT(projectTemplate,
           OPI_INTERFACE(newWizard)
           )

OPI_OBJECT(options,
           OPI_INTERFACE(showCfgDialg, "itemName")
           OPI_INTERFACE(configSaved)
           )

OPI_OBJECT(workspace,
           OPI_INTERFACE(expandAll)
           OPI_INTERFACE(foldAll)
           )
OPI_OBJECT(session,
           OPI_INTERFACE(readyToSaveSession)
           OPI_INTERFACE(sessionStatusChanged)
           OPI_INTERFACE(sessionLoaded, "session")
           OPI_INTERFACE(sessionCreated, "session")
           OPI_INTERFACE(sessionRenamed, "oldName", "newName")
           OPI_INTERFACE(sessionRemoved, "session")
           )
OPI_OBJECT(ai,
           OPI_INTERFACE(LLMChanged)
           )

struct AnalysedData
{
    struct TokenMap
    {
        newlsp::Enum::SemanticTokenTypes::type_value semanticTokenType;
        newlsp::Enum::SemanticTokenModifiers::type_value semanticTokenModifier;
        std::vector<float> result;
    };
    std::vector<std::string> rules;
    std::vector<TokenMap> tokenMaps;
};
Q_DECLARE_METATYPE(AnalysedData);

enum FindType{
    Previous = 0,
    Next
};
Q_DECLARE_METATYPE(FindType);

enum RepalceType{
    Repalce = 0,
    FindAndReplace,
    RepalceAll
};
Q_DECLARE_METATYPE(RepalceType);

extern const QString T_MENU;
extern const QString T_FILEBROWSER;
extern const QString T_DEBUGGER;
extern const QString T_BUILDER;
extern const QString T_WORKSPACE;
extern const QString T_RECENT;
extern const QString T_SYMBOL;
extern const QString T_PROCESSMESSAGE;
extern const QString T_FIND;
extern const QString T_COLLABORATORS;

extern const QString D_BUILD_STATE;
extern const QString D_ITEM_MENU_BUILD;
extern const QString D_BUILD_TARGET;
extern const QString D_SHOW;
extern const QString D_HIDE;
extern const QString D_ADDTEXT;
extern const QString D_BUILD_COMMAND;
extern const QString D_MENU_BUILD;
extern const QString D_MENU_REBUILD;
extern const QString D_MENU_CLEAN;
extern const QString D_OPEN_REPOS;

extern const QString D_SEARCH;
extern const QString D_REPLACE;
extern const QString D_OPENFILE;

extern const QString P_ACTION_TEXT;    // value QString
extern const QString P_FILELINE;       // value int
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

#endif // EVENTDEFINITIONS_H
