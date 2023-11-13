// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include "projectgenerate.h"
#include <DAbstractDialog>

using namespace templateMgr;

class MainDialogPrivate;
class MainDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

signals:

private slots:

private:
    void setupUI(TemplateVector &templateVec);
    void generate(const PojectGenParam &param);

    MainDialogPrivate *const d;
};

#endif // MAINDIALOG_H
