// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PYTHONPROJECTGENERATOR_H
#define PYTHONPROJECTGENERATOR_H

#include "pythonasynparse.h"
#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class PythonProjectGeneratorPrivate;
class PythonProjectGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    PythonProjectGeneratorPrivate *const d;
public:
    explicit PythonProjectGenerator();
    virtual ~PythonProjectGenerator();
    static QString toolKitName() { return "directory"; }
    virtual QStringList supportLanguages() override;
    virtual QDialog* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void doProjectChildsModified(const QList<QStandardItem*> &info);
    void doPythonCleanMenu();
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
};

#endif // PYTHONPROJECTGENERATOR_H
