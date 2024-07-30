// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJAPROJECTGENERATOR_H
#define NINJAPROJECTGENERATOR_H

#include "ninjaasynparse.h"
#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class NinjaProjectGeneratorPrivate;
class NinjaProjectGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    NinjaProjectGeneratorPrivate *const d;
public:
    explicit NinjaProjectGenerator();
    virtual ~NinjaProjectGenerator() override;
    static QString toolKitName() { return "ninja"; }
    virtual QStringList supportLanguages() override;
    virtual QStringList supportFileNames() override;
    virtual DWidget* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual void acceptConfigure() override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void doProjectChildsModified(const QList<QStandardItem*> &items);
};

#endif // NINJAPROJECTGENERATOR_H
