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
#ifndef GRADLEGENERATOR_H
#define GRADLEGENERATOR_H

#include "gradleasynparse.h"
#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class GradleGeneratorPrivate;
class GradleGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    GradleGeneratorPrivate *const d;
public:
    explicit GradleGenerator();
    virtual ~GradleGenerator();
    static QString toolKitName() { return "gradle"; }
    virtual QWidget* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void doProjectChildsModified(const dpfservice::ParseInfo<QList<QStandardItem*>> &info);
    void doGradleGeneratMenu(const QString &program, const QStringList &args, const QString &workdir);
    void doGradleCleanMenu();
    void doGradleSplitTasksOutput(const QByteArray &array);
    void doGradleTaskActionTriggered();
};

#endif // GRADLEGENERATOR_H
