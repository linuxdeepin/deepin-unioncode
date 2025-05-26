// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DDialog>
#include <DTableView>

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class LLMInfo;
class AddModelDialogPrivate;
class ModelConfigDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    ModelConfigDialog(QWidget *parent = nullptr);
    ~ModelConfigDialog();
    LLMInfo getLLmInfo();
    void setLLmInfo(const LLMInfo &llmInfo);

private:
    AddModelDialogPrivate *d;
};
