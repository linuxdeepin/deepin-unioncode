// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PYTHONPROJECTGENERATOR_H
#define PYTHONPROJECTGENERATOR_H

#include "services/project/projectservice.h"
#include "services/project/directorygenerator.h"

#include <QObject>
#include <QDomDocument>

class PythonProjectGeneratorPrivate;
class PythonProjectGenerator : public dpfservice::DirectoryGenerator
{
    Q_OBJECT
    PythonProjectGeneratorPrivate *const d;

public:
    explicit PythonProjectGenerator();
    virtual ~PythonProjectGenerator();
    static QString toolKitName() { return "python"; }
    virtual QString configureKitName() override { return "python"; }
    virtual QStringList supportLanguages() override;
    virtual DWidget *configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QMenu *createItemMenu(const QStandardItem *item) override;

private slots:
    void doPythonCleanMenu();
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
};

#endif   // PYTHONPROJECTGENERATOR_H
