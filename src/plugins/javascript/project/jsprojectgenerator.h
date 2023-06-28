// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSProjectGenerator_H
#define JSProjectGenerator_H

#include "jsasynparse.h"
#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class JSProjectGeneratorPrivate;
class JSProjectGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    JSProjectGeneratorPrivate *const d;
public:
    explicit JSProjectGenerator();
    virtual ~JSProjectGenerator() override;
    static QString toolKitName() { return "jsdirectory"; }
    virtual QStringList supportLanguages() override;
    virtual QDialog* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void doProjectChildsModified(const QList<QStandardItem*> &info);
    void doJSCleanMenu();
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
};

#endif // JSProjectGenerator_H
