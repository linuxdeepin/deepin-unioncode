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
    virtual bool canOpenProject(const QString &language, const QString &workspace);
    virtual void doProjectOpen(const QString &language, const QString &actionText, const QString &workspace);
    virtual QDialog* configureWidget(const QString &language, const QString &workspace);
    virtual bool configure(const ProjectInfo &projectInfo);
    virtual QStandardItem *createRootItem(const ProjectInfo &info);
    virtual void removeRootItem(QStandardItem *root);
    virtual QMenu* createItemMenu(const QStandardItem *item);
    static QStandardItem *root(QStandardItem *child);
    static const QModelIndex root(const QModelIndex &child);

protected:
    template<class T> struct ParseInfo
    {
        T result;
        bool isNormal = true;
    };
};
} // namespace dpfservice

#endif // PROJECTGENERATOR_H
