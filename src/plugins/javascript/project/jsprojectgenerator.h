/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
    void doJSSplitTasksOutput(const QByteArray &array);
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
};

#endif // JSProjectGenerator_H
