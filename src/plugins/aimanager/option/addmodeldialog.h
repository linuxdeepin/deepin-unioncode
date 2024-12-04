// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DDialog>
#include <DTableView>

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class LLMInfo;
class AddModelDialogPrivate;
class AddModelDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    AddModelDialog(QWidget *parent = nullptr);
    ~AddModelDialog();
    LLMInfo getNewLLmInfo(); 

private:
    AddModelDialogPrivate *d;
};
