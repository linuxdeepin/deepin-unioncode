// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "constants.h"

class MainControllerPrivate;
class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();

    bool search(const SearchParams &params);
    bool replace(const ReplaceParams &params);
    void stop();
    FindItemList takeAll();
    
Q_SIGNALS:
    void matched();
    void searchFinished();
    void replaceFinished();

private:
    MainControllerPrivate *const d;
};

#endif // MAINCONTROLLER_H
