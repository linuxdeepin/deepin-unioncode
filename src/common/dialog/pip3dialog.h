// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PIP3DIALOG_H
#define PIP3DIALOG_H

#include "processdialog.h"

class Pip3Dialog : public ProcessDialog
{
    Q_OBJECT
public:
    Pip3Dialog(QWidget *parent = nullptr);
    void install(const QString &packageName);
    void uninstall(const QString &packageName);
protected:
    virtual void doShowStdErr(const QByteArray &array);
};

#endif // PIP3DIALOG_H
