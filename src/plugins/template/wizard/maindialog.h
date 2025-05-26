// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include "detailwidget.h"
#include "projectgenerate.h"
#include <DAbstractDialog>
#include <DWidget>
#include <DLineEdit>
#include <DTreeView>

using namespace templateMgr;

class MainDialogPrivate;
class MainDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit MainDialog(DWidget *parent = nullptr);
    ~MainDialog();
    void generate(const PojectGenParam &param);

signals:

private slots:
    void closeMainWindow();  // 声明用于关闭主窗口的槽函数
private:
    void setupUI(TemplateVector &templateVec);
    MainDialogPrivate *const d;

};

#endif // MAINDIALOG_H
