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
#ifndef ADAPTERCONFIGURE_H
#define ADAPTERCONFIGURE_H

#include "common/common.h"

#include <QWidget>
#include <QCheckBox>
#include <QButtonGroup>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class ProcConfigure : public QWidget
{
    Q_OBJECT
public:
    explicit ProcConfigure(QWidget *parent = nullptr);
    QLineEdit *LineEditlauchCmd(){ return lauchCmd; }

protected:
    QLineEdit *lauchCmd {nullptr};
    QVBoxLayout *vLayout {nullptr};
};

class SockConfigure : public ProcConfigure
{
    Q_OBJECT
public:
    explicit SockConfigure(QWidget *parent = nullptr);
    QLineEdit *lineEditIP() {return ip;}
    QLineEdit *lineEditPort() {return port;}

protected:
    QLineEdit *ip {nullptr};
    QLineEdit *port {nullptr};
};

class AdapterConfigure : public PageWidget
{
    Q_OBJECT
public:
    explicit AdapterConfigure(QWidget *parent = nullptr);
    virtual void saveConfig(){qInfo() <<__FUNCTION__;}
    virtual void readConfig(){qInfo() <<__FUNCTION__;}
private slots:
    void clicked();

protected:
    QCheckBox *checkBoxProc {nullptr};
    QCheckBox *checkBoxSock {nullptr};
    QVBoxLayout *mainVLayout {nullptr};
    QButtonGroup *checkBoxGroup {nullptr};
    QHash<QCheckBox*, ProcConfigure*> sections {};
};

#endif // ADAPTERCONFIGURE_H
