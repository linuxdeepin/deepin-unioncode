// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEBROWSERRECEIVER_H
#define FILEBROWSERRECEIVER_H

#include <framework/framework.h>

class FileBrowserReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<FileBrowserReceiver>
{
    friend class dpf::AutoEventHandlerRegister<FileBrowserReceiver>;
public:
    explicit FileBrowserReceiver(QObject * parent = nullptr);
    static Type type();
    static QStringList topics();

private:
    void eventProcess(const dpf::Event &event) override;
};

#endif // FILEBROWSERRECEIVER_H
