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
#ifndef HANDLERBACKEND_H
#define HANDLERBACKEND_H

#include "handler.h"
#include "settinginfo.h"

class HandlerBackend : public Handler
{
    Q_OBJECT
public:

    HandlerBackend(const SettingInfo &info);

    virtual void bind(QProcess *qIODevice);
    virtual void filterData(const QByteArray &array);
    virtual void response(const QByteArray &array);
    virtual bool exitNotification(const QJsonObject &obj);

    virtual SettingInfo getSettingInfo() {return info;}

private slots:
    void doFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    SettingInfo info;
};

#endif // HANDLERBACKEND_H
