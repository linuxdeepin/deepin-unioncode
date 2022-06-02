/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#ifndef FINDRECEIVER_H
#define FINDRECEIVER_H

#include <framework/framework.h>

class FindReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<FindReceiver>
{
    Q_OBJECT
public:
    explicit FindReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event) override;
};

class FindEventTransmit : public QObject
{
    Q_OBJECT
public:
    static FindEventTransmit* getInstance();

signals:
    void sendProjectPath(const QString &projectPath, const QString &language);
    void sendRemovedProject(const QString &projectPath);
    void sendCurrentEditFile(const QString &filePath, bool actived);

private:
    explicit FindEventTransmit(QObject *parent = nullptr);
    ~FindEventTransmit();
};


#endif // FINDRECEIVER_H
