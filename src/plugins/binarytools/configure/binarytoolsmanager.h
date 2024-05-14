// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSMANAGER_H
#define BINARYTOOLSMANAGER_H

#include "constants.h"
#include "common/widget/outputpane.h"

namespace dpfservice {
class WindowService;
}
class AbstractAction;

class BinaryToolsManager : public QObject
{
    Q_OBJECT
public:
    static BinaryToolsManager *instance();

    void save();
    void setTools(const QMap<QString, QList<ToolInfo>> &dataMap);
    ToolInfo findTool(const QString &id);
    QMap<QString, QList<ToolInfo>> tools();

    void executeTool(const QString &id);
    void checkAndAddToToolbar(const QMap<QString, QList<ToolInfo>> &tools);

private:
    bool checkCommandExists(const QString &command);
    void addToToolBar(const ToolInfo &tool);
    void printOutput(const QString &content, OutputPane::OutputFormat format);

private:
    QMap<QString, QList<ToolInfo>> allTools;
    dpfservice::WindowService *windowSrv { nullptr };
    QMap<QString, AbstractAction *> actMap;
};

#endif   // BINARYTOOLSMANAGER_H
