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
#ifndef CMAKEPROJECTGENERATOR_H
#define CMAKEPROJECTGENERATOR_H

#include "services/project/projectservice.h"
#include "services/builder/builderservice.h"

#include <QObject>
#include <QDomDocument>

class CmakeProjectGeneratorPrivate;
class CmakeProjectGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    CmakeProjectGeneratorPrivate *const d;
public:
    explicit CmakeProjectGenerator();
    ~CmakeProjectGenerator() override;
    static QString toolKitName() { return "cmake"; }
    virtual QStringList supportLanguages() override;
    virtual QStringList supportFileNames() override;
    virtual QDialog* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void actionTriggered();
    void setRootItemToView(QStandardItem *root);
    void doBuildCmdExecuteEnd(const BuildCommandInfo &info, int status);
    void doCmakeFileNodeChanged(QStandardItem *root, const QPair<QString, QStringList> &files);
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
    void recursionRemoveItem(QStandardItem *item);

private:
    QMutex mutex;
    QStandardItem *rootItem = nullptr;
};

#endif // CMAKEPROJECTGENERATOR_H
