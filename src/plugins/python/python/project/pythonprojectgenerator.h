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
