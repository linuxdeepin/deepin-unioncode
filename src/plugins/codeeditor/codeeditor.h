// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include <framework/framework.h>

class QTabWidget;
class AbstractWidget;
class QSplitter;
class WorkspaceWidget;
class CodeEditor : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "codeeditor.json")
public:
    CodeEditor();
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

private:
    void initActions();
    void initButtonBox();
    void initEditorService();
    void initWindowService();
    void initOptionService();
    void registerVariables();

private:
    WorkspaceWidget *workspaceWidget {nullptr};
};

#endif   // COREPLUGIN_H
