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
#ifndef MAVENGENERATOR_H
#define MAVENGENERATOR_H

#include "mavenasynparse.h"
#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class MavenGeneratorPrivate;
class MavenGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    MavenGeneratorPrivate *const d;
public:
    explicit MavenGenerator();
    virtual ~MavenGenerator();
    static QString toolKitName() { return "maven"; }
    virtual QDialog* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;
private slots:
    void doProjectChildsModified(const dpfservice::ParseInfo<QList<QStandardItem*>> &info);
    void doAddMavenMeue(const dpfservice::ParseInfo<dpfservice::ProjectActionInfos> &info);
    void doActionTriggered();
};

#endif // MAVENGENERATOR_H
