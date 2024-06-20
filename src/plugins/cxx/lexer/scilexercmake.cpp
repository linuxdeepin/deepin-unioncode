// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scilexercmake.h"

#include <DGuiApplicationHelper>

#include <QColor>
#include <QFont>
#include <QFileInfo>

DGUI_USE_NAMESPACE

SciLexerCMake::SciLexerCMake(QObject *parent)
    : AbstractLexerProxy(parent)
{
}

const char *SciLexerCMake::language() const
{
    return "CMake";
}

const char *SciLexerCMake::lexer() const
{
    return "cmake";
}

QString SciLexerCMake::description(int style) const
{
    switch (style) {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case String:
        return tr("String");

    case StringLeftQuote:
        return tr("Left quoted string");

    case StringRightQuote:
        return tr("Right quoted string");

    case Function:
        return tr("Function");

    case Variable:
        return tr("Variable");

    case Label:
        return tr("Label");

    case KeywordSet3:
        return tr("User defined");

    case BlockWhile:
        return tr("WHILE block");

    case BlockForeach:
        return tr("FOREACH block");

    case BlockIf:
        return tr("IF block");

    case BlockMacro:
        return tr("MACRO block");

    case StringVariable:
        return tr("Variable within a string");

    case Number:
        return tr("Number");
    }

    return QString();
}

bool SciLexerCMake::isSupport(const QString &file) const
{
    QFileInfo info(file);
    return info.fileName().compare("CMakeLists.txt") == 0;
}

QColor SciLexerCMake::defaultColor(int style) const
{
    bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    switch (style) {
    case Default:
    case KeywordSet3:
        return isDarkTheme ? QColor("#45c6d6") : QColor("#808000");

    case Comment:
        return isDarkTheme ? QColor("#a8abb0") : QColor("#008000");

    case String:
    case StringLeftQuote:
    case StringRightQuote:
        return isDarkTheme ? QColor("#d69545") : QColor("#008000");

    case Function:
    case BlockWhile:
    case BlockForeach:
    case BlockIf:
    case BlockMacro:
        return isDarkTheme ? QColor("#d6cf9a") : QColor("#00677c");

    case Variable:
        return isDarkTheme ? QColor("#d6cf9a") : QColor("#800000");

    case Label:
    case StringVariable:
        return isDarkTheme ? QColor("#d6c540") : QColor("#800000");

    case Number:
        return isDarkTheme ? QColor("#8a602c") : QColor("#000080");
    }

    return AbstractLexerProxy::defaultColor(style);
}

QFont SciLexerCMake::defaultFont(int style) const
{
    return AbstractLexerProxy::defaultFont(style);
}

QColor SciLexerCMake::defaultPaper(int style) const
{
    return AbstractLexerProxy::defaultPaper(style);
}

const char *SciLexerCMake::keywords(int set) const
{
    if (set == 1)
        return "add_custom_command add_custom_target add_definitions "
               "add_dependencies add_executable add_library add_subdirectory "
               "add_test aux_source_directory build_command build_name "
               "cmake_minimum_required configure_file create_test_sourcelist "
               "else elseif enable_language enable_testing endforeach endif "
               "endmacro endwhile exec_program execute_process "
               "export_library_dependencies file find_file find_library "
               "find_package find_path find_program fltk_wrap_ui foreach "
               "get_cmake_property get_directory_property get_filename_component "
               "get_source_file_property get_target_property get_test_property "
               "if include include_directories include_external_msproject "
               "include_regular_expression install install_files "
               "install_programs install_targets link_directories link_libraries "
               "list load_cache load_command macro make_directory "
               "mark_as_advanced math message option output_required_files "
               "project qt_wrap_cpp qt_wrap_ui remove remove_definitions "
               "separate_arguments set set_directory_properties "
               "set_source_files_properties set_target_properties "
               "set_tests_properties site_name source_group string "
               "subdir_depends subdirs target_link_libraries try_compile try_run "
               "use_mangled_mesa utility_source variable_requires "
               "vtk_make_instantiator vtk_wrap_java vtk_wrap_python vtk_wrap_tcl "
               "while write_file";

    if (set == 2)
        return "ABSOLUTE ABSTRACT ADDITIONAL_MAKE_CLEAN_FILES ALL AND APPEND "
               "ARGS ASCII BEFORE CACHE CACHE_VARIABLES CLEAR COMMAND COMMANDS "
               "COMMAND_NAME COMMENT COMPARE COMPILE_FLAGS COPYONLY DEFINED "
               "DEFINE_SYMBOL DEPENDS DOC EQUAL ESCAPE_QUOTES EXCLUDE "
               "EXCLUDE_FROM_ALL EXISTS EXPORT_MACRO EXT EXTRA_INCLUDE "
               "FATAL_ERROR FILE FILES FORCE FUNCTION GENERATED GLOB "
               "GLOB_RECURSE GREATER GROUP_SIZE HEADER_FILE_ONLY HEADER_LOCATION "
               "IMMEDIATE INCLUDES INCLUDE_DIRECTORIES INCLUDE_INTERNALS "
               "INCLUDE_REGULAR_EXPRESSION LESS LINK_DIRECTORIES LINK_FLAGS "
               "LOCATION MACOSX_BUNDLE MACROS MAIN_DEPENDENCY MAKE_DIRECTORY "
               "MATCH MATCHALL MATCHES MODULE NAME NAME_WE NOT NOTEQUAL "
               "NO_SYSTEM_PATH OBJECT_DEPENDS OPTIONAL OR OUTPUT OUTPUT_VARIABLE "
               "PATH PATHS POST_BUILD POST_INSTALL_SCRIPT PREFIX PREORDER "
               "PRE_BUILD PRE_INSTALL_SCRIPT PRE_LINK PROGRAM PROGRAM_ARGS "
               "PROPERTIES QUIET RANGE READ REGEX REGULAR_EXPRESSION REPLACE "
               "REQUIRED RETURN_VALUE RUNTIME_DIRECTORY SEND_ERROR SHARED "
               "SOURCES STATIC STATUS STREQUAL STRGREATER STRLESS SUFFIX TARGET "
               "TOLOWER TOUPPER VAR VARIABLES VERSION WIN32 WRAP_EXCLUDE WRITE "
               "APPLE MINGW MSYS CYGWIN BORLAND WATCOM MSVC MSVC_IDE MSVC60 "
               "MSVC70 MSVC71 MSVC80 CMAKE_COMPILER_2005 OFF ON";

    return nullptr;
}

void SciLexerCMake::refreshProperties()
{
    emit propertyChanged("fold.at.else", "1");
}
