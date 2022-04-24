/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef RUNTIMEMANAGER_H
#define RUNTIMEMANAGER_H

#include <QObject>

class RuntimeWidget;
class RuntimeManager : public QObject
{
    Q_OBJECT
public:

    static RuntimeManager *instance();
    explicit RuntimeManager(QObject *parent = nullptr);

    RuntimeWidget *getRuntimeWidget() const;
    void showConfigureProjDlg(QString &projectPath);

signals:

public slots:

private:
    RuntimeWidget *runtimeWidget = nullptr;
};

#endif // RUNTIMEMANAGER_H
