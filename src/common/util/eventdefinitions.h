// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTDEFINITIONS_H
#define EVENTDEFINITIONS_H

#include "common/lsp/protocol/newprotocol.h"
#include "framework.h"

#include <QString>

OPI_OBJECT(recent,
           OPI_INTERFACE(saveOpenedProject, "kitName", "language", "workspace")
           OPI_INTERFACE(saveOpenedFile, "filePath")
           )

OPI_OBJECT(project,
           // in
           OPI_INTERFACE(openProject, "kitName", "language", "workspace")
           OPI_INTERFACE(activeProject, "kitName", "language", "workspace")
           // out
           OPI_INTERFACE(activedProject, "projectInfo")
           OPI_INTERFACE(deletedProject, "projectInfo")
           OPI_INTERFACE(createdProject, "projectInfo")
           )

OPI_OBJECT(debugger,
           OPI_INTERFACE(prepareDebugProgress, "message")
           OPI_INTERFACE(prepareDebugDone, "succeed", "message")
           OPI_INTERFACE(executeStart)
           )

OPI_OBJECT(editor,
           // in
           OPI_INTERFACE(openFile, "filePath")
           OPI_INTERFACE(back)
           OPI_INTERFACE(forward)
           OPI_INTERFACE(gotoLine, "filePath", "line")

           OPI_INTERFACE(openFileWithKey, "workspace", "language", "filePath")
           OPI_INTERFACE(jumpToLineWithKey, "workspace", "language", "filePath", "line")
           // (AnnotationInfo)annInfo
           OPI_INTERFACE(setAnnotation, "filePath", "line", "title", "annInfo")
           OPI_INTERFACE(cleanAnnotation, "filePath", "title")
           OPI_INTERFACE(setDebugLine, "filePath", "line")
           OPI_INTERFACE(removeDebugLine)
           OPI_INTERFACE(setLineBackground, "filePath", "line", "color")
           OPI_INTERFACE(deleteLineBackground, "filePath", "line")
           OPI_INTERFACE(cleanLineBackground, "filePath")
           OPI_INTERFACE(setModifiedAutoReload, "filePath", "flag")
           OPI_INTERFACE(addBreakpoint, "filePath", "line")
           OPI_INTERFACE(removeBreakpoint, "filePath", "line")
           // (FindType)findType
           OPI_INTERFACE(searchText, "text", "findType")
           // (FindType)repalceType
           OPI_INTERFACE(replaceText, "text", "target", "repalceType")
           // out
           OPI_INTERFACE(fileOpened, "filePath")
           OPI_INTERFACE(fileClosed, "filePath")
           OPI_INTERFACE(switchedFile, "filePath")
           OPI_INTERFACE(breakpointAdded, "filePath", "line")
           OPI_INTERFACE(breakpointRemoved, "filePath", "line")
           OPI_INTERFACE(switchContext, "name")
           OPI_INTERFACE(switchWorkspace, "name")
           OPI_INTERFACE(contextMenu, "menu")
           OPI_INTERFACE(marginMenu, "menu")
           OPI_INTERFACE(keyPressEvent, "event")
           )

OPI_OBJECT(symbol,
           OPI_INTERFACE(parse, "workspace", "language", "storage")
           OPI_INTERFACE(parseDone, "workspace", "language", "storage", "success")
           )

OPI_OBJECT(navigation,
           OPI_INTERFACE(doSwitch, "actionText")
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

struct AnnotationInfo
{
    struct RoleElem
    {
        QString display;
        int code;
        bool operator == (const RoleElem &elem)
        {
            return display == elem.display
                    && code == elem.code;
        }
    };
    enum_def(Role, RoleElem)
    {
        enum_exp Note = {"Note", 767};
        enum_exp Warning = {"Warning", 766};
        enum_exp Error = {"Error", 765};
        enum_exp Fatal = {"Fatal", 764};
    };
    Role::type_value role;
    QString text;
    AnnotationInfo(const QString &text)
        : text(text) { role = Role::get()->Note;}
    AnnotationInfo(Role::type_value role, const QString &text)
        : role(role), text(text){}
    AnnotationInfo() :role(Role::get()->Note), text(""){}
    AnnotationInfo(const AnnotationInfo &info)
        : role(info.role), text(info.text){}
    bool operator == (const AnnotationInfo &info) {
        return role == info.role
                && text == info.text;
    }
};
Q_DECLARE_METATYPE(AnnotationInfo);

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
