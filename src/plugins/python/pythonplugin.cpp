// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythonplugin.h"
#include "python/pythongenerator.h"
#include "python/project/pythonprojectgenerator.h"
#include "python/option/optionpythongenerator.h"
#include "lexer/scilexerpython.h"
#include "installer/pipinstaller.h"

#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"
#include "services/option/optiondatastruct.h"
#include "services/editor/editorservice.h"
#include "services/window/windowservice.h"

using namespace dpfservice;

void PythonPlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool PythonPlugin::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *languageService = ctx.service<LanguageService>(LanguageService::name());
    if (languageService) {
        QString errorString;
        bool ret = languageService->regClass<PythonGenerator>(PythonGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<PythonGenerator>(PythonGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }
    }

    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        QString errorString;
        projectService->implGenerator<PythonProjectGenerator>(PythonProjectGenerator::toolKitName(), &errorString);
    }

    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (!optionService) {
        qCritical() << "Failed, not found OptionPython service!";
        abort();
    }
    optionService->implGenerator<OptionPythonGenerator>(option::GROUP_LANGUAGE, OptionPythonGenerator::kitName());

    registEditorService();
    registerPIPInstaller();
    return true;
}

dpf::Plugin::ShutdownFlag PythonPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

void PythonPlugin::registEditorService()
{
    auto &ctx = dpfInstance.serviceContext();
    EditorService *editorSvc = ctx.service<EditorService>(EditorService::name());
    if (!editorSvc)
        return;

    SciLexerPython *lexerPython = new SciLexerPython;
    editorSvc->registerSciLexerProxy(lexerPython->language(), lexerPython);
}

void PythonPlugin::registerPIPInstaller()
{
    auto winSrv = dpfGetService(WindowService);
    if (!winSrv)
        return;

    auto installer = new PIPInstaller(qApp);
    winSrv->registerInstaller("pip", installer);
}
