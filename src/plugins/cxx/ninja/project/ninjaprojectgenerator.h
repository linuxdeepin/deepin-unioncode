// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJAPROJECTGENERATOR_H
#define NINJAPROJECTGENERATOR_H

#include "services/project/projectservice.h"
#include "services/project/directorygenerator.h"

#include <QObject>
#include <QDomDocument>

class NinjaProjectGenerator : public dpfservice::DirectoryGenerator
{
    Q_OBJECT
public:
    explicit NinjaProjectGenerator();
    virtual ~NinjaProjectGenerator() override;
    static QString toolKitName() { return "ninja"; }
    virtual QString configureKitName() { return "ninja"; }
    virtual QStringList supportLanguages() override;
    virtual QStringList supportFileNames() override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;
};

#endif // NINJAPROJECTGENERATOR_H
