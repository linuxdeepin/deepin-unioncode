// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DDialog>
#include <DTableView>

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class AttachInfoDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    AttachInfoDialog(QWidget *parent = nullptr);
    ~AttachInfoDialog(){}

signals:
    void attachToProcessId(const QString &pid);

private:
    void initUi();
    void initButton();
    void updateProcess();

    Dtk::Widget::DTableView *view = nullptr;
    QStandardItemModel *model = nullptr;
    QSortFilterProxyModel *proxy = nullptr;
};
