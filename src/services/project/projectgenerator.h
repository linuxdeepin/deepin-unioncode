// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTGENERATOR_H
#define PROJECTGENERATOR_H

#include "projectinfo.h"

#include "common/common.h"

#include <framework/framework.h>

#include <QStandardItem>
#include <QMenu>

class QFileDialog;
namespace dpfservice {

class ProjectGenerator : public Generator
{
    Q_OBJECT
public:
    ProjectGenerator(){}
    virtual QStringList supportLanguages();
    virtual QStringList supportFileNames();
    virtual QAction* openProjectAction(const QString &language, const QString &actionText);
    virtual bool canOpenProject(const QString &kitName, const QString &language,
                                const QString &workspace);
    virtual bool isOpenedProject(const QString &kitName, const QString &language,
                                 const QString &workspace);
    virtual void doProjectOpen(const QString &language, const QString &actionText,
                               const QString &workspace);
    virtual QDialog* configureWidget(const QString &language, const QString &workspace);
    virtual bool configure(const ProjectInfo &projectInfo);
    virtual QStandardItem *createRootItem(const ProjectInfo &info);
    virtual void removeRootItem(QStandardItem *root);
    virtual QMenu* createItemMenu(const QStandardItem *item);
    virtual void createDocument(const QStandardItem *item, const QString &filePath);
    static QStandardItem *root(QStandardItem *child);
    static const QModelIndex root(const QModelIndex &child);

signals:
    void itemChanged(QStandardItem *item, const QList<QStandardItem*> &newChilds);
};
} // namespace dpfservice

#endif // PROJECTGENERATOR_H
