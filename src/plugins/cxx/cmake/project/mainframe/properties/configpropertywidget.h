/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#ifndef CONFIGPROPERTYDIALOG_H
#define CONFIGPROPERTYDIALOG_H

#include "services/project/projectinfo.h"

#include <QDialog>

class ConfigPropertyWidgetPrivate;
class ConfigPropertyWidget : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigPropertyWidget(const QString &language,
                                  const QString &workspace,
                                  QDialog *parent = nullptr);
    virtual ~ConfigPropertyWidget();

signals:
    void configureDone(const dpfservice::ProjectInfo &info);

private:
    void closeWidget();
    ConfigPropertyWidgetPrivate *const d;
};

#endif // CONFIGPROPERTYDIALOG_H
