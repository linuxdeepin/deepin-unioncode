// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTRECEIVER_H
#define PROJECTRECEIVER_H

#include "services/project/projectservice.h"

#include <framework/framework.h>

class DStandardItem;
class SymbolReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<SymbolReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<SymbolReceiver>;
public:
    explicit SymbolReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

class ProjectProxy : public QObject
{
    Q_OBJECT
    ProjectProxy(){}
    ProjectProxy(const ProjectProxy&) = delete;

public:
    static ProjectProxy* instance() {
        static ProjectProxy ins;
        return &ins;
    }

signals:
    void toSetSymbolRootItem(const dpfservice::ProjectInfo &info);
};

#endif // PROJECTRECEIVER_H
