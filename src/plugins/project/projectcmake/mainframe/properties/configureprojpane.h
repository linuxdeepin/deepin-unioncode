/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef CONFIGUREPROJPANE_H
#define CONFIGUREPROJPANE_H


#include "configutil.h"

#include "services/project/projectinfo.h"

#include <QWidget>

class ConfigureProjPanePrivate;
class ConfigureProjPane : public QWidget
{
    Q_OBJECT
public:
    ConfigureProjPane(const QString &language,
                               const QString &projectPath,
                               QWidget *parent = nullptr);
    ~ConfigureProjPane();

signals:
    void configureDone(const dpfservice::ProjectInfo &info);

public slots:
    void slotConfigureDone();

private:
    void setupUI();
    void resetUI();
    void updateUI();
    void configProject(const config::ConfigureParam *param);

    ConfigureProjPanePrivate *const d;
};

#endif   // CONFIGUREPROJPANE_H
