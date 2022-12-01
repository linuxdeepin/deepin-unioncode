/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CMAKEGENERATOR_H
#define CMAKEGENERATOR_H

#include "services/project/projectservice.h"
#include "services/builder/builderservice.h"

#include <QObject>
#include <QDomDocument>

class CmakeGeneratorPrivate;
class CmakeGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    CmakeGeneratorPrivate *const d;
public:
    explicit CmakeGenerator();
    virtual ~CmakeGenerator();
    static QString toolKitName() { return "cmake"; }
    virtual QStringList supportLanguages() override;
    virtual QStringList supportFileNames() override;
    virtual QDialog* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

signals:
    void createRootItemAsynEnd(QStandardItem *root);

private slots:
    void actionTriggered();
    void setRootItemToView(QStandardItem *root);
    void doBuildCmdExecuteEnd(const BuildCommandInfo &info, int status);
    void doCmakeFileNodeChanged(QStandardItem *root, const QPair<QString, QStringList> &files);
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
    void recursionRemoveItem(QStandardItem *item);

private:
    // cmake CDT4 options
    QStandardItem *cdt4FindItem(QStandardItem *rootItem, QString &name);
    QStandardItem *cdt4FindParentItem(QStandardItem *rootItem, QString &name);
    QHash<QString, QString> cdt4Subporjects(QStandardItem *rootItem);
    void cdt4TargetsDisplayOptimize(QStandardItem *item, const QHash<QString, QString> &subprojectsMap);
    void cdt4SubprojectsDisplayOptimize(QStandardItem *item);
    QDomDocument cdt4LoadMenuXmlDoc(const QString &buildFolder);

    QMutex mutex;
};

#endif // CMAKEGENERATOR_H
