// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERATEDIALOG_H
#define GENERATEDIALOG_H

#include <DDialog>

class GenerateDialogPrivate;
class GenerateDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    GenerateDialog(QWidget *parent = nullptr);

private:
    GenerateDialogPrivate *d;

    bool checkFiledsInfo();
    void initUi();
    void initConnection();
    void generate();
};

#endif   // GENERATEDIALOG_H
