// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSProjectGenerator_H
#define JSProjectGenerator_H

#include "services/project/projectservice.h"
#include "services/project/directorygenerator.h"

#include <QObject>

class JSProjectGeneratorPrivate;
class JSProjectGenerator : public dpfservice::DirectoryGenerator
{
    Q_OBJECT
    JSProjectGeneratorPrivate *const d;
public:
    explicit JSProjectGenerator();
    virtual ~JSProjectGenerator() override;
    static QString toolKitName() { return "javascript"; }
    virtual QString configureKitName() override { return "javascript"; }
    virtual QStringList supportLanguages() override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void doJSCleanMenu();
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
};

#endif // JSProjectGenerator_H
