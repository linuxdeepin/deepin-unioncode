// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
