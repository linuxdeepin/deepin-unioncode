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
#ifndef RUNCONFIGPANE_H
#define RUNCONFIGPANE_H

#include "configutil.h"

#include <QWidget>

class RunConfigPanePrivate;
class RunConfigPane : public QWidget
{
    Q_OBJECT
public:
    explicit RunConfigPane(QWidget *parent = nullptr);
    ~RunConfigPane();

    void bindValues(config::RunParam *runParam);

signals:

public slots:

private:
    void setupUi();

    RunConfigPanePrivate *const d;
};

#endif // RUNCONFIGPANE_H
