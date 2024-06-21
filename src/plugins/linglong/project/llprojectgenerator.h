// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLPROJECTGENERATOR_H
#define LLPROJECTGENERATOR_H

#include "llasynparse.h"
#include "llglobal.h"
#include "services/project/projectservice.h"

class LLProjectGeneratorPrivate;
class LLProjectGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
public:
    explicit LLProjectGenerator();
    virtual ~LLProjectGenerator();

    static QString toolKitName() { return LL_TOOLKIT; }
    virtual QStringList supportLanguages() override;
    virtual QDialog *configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem *root) override;
    virtual QMenu *createItemMenu(const QStandardItem *item) override;

private slots:
    void doProjectChildsModified(const QList<QStandardItem *> &info);
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);

private:
    LLProjectGeneratorPrivate *const d;
};

#endif   // LLPROJECTGENERATOR_H
