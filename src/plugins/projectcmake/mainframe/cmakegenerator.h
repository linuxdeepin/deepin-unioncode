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

#include <QObject>
#include <QDomDocument>

class CMakeGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    QProcess process;
public:
    explicit CMakeGenerator();
    static QString toolKitName() { return "cmake"; }
    virtual bool configure(const QString &projectPath) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void processReadAll();
    void processFinished(int code, QProcess::ExitStatus status);
    void actionTriggered();
private:
    // cmake CDT4 options
    QStandardItem *cdt4FindItem(QStandardItem *rootItem, QString &name);
    QStandardItem *cdt4FindParentItem(QStandardItem *rootItem, QString &name);
    QHash<QString, QString> cdt4Subporjects(QStandardItem *rootItem);
    QStandardItem *cdt4DisplayOptimize(QStandardItem *rootItem);
    void cdt4TargetsDisplayOptimize(QStandardItem *item, const QHash<QString, QString> &subprojectsMap);
    void cdt4SubprojectsDisplayOptimize(QStandardItem *item);
    QDomDocument cdt4LoadProjectXmlDoc(const QString &buildFolder);
    QDomDocument cdt4LoadMenuXmlDoc(const QString &buildFolder);
};

#endif // CMAKEGENERATOR_H
