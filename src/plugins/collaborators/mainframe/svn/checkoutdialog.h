// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <DDialog>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>

DWIDGET_USE_NAMESPACE
class QGridLayout;
class QVBoxLayout;
class CheckoutDialog : public DDialog
{
    Q_OBJECT
public:
    explicit CheckoutDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void checkoutRepos(const QString &remote, const QString &local,
                       const QString &user, const QString &passwd);

private:
    DLabel *labelRepos{nullptr};
    DLabel *labelLocal{nullptr};
    DLabel *labelUser{nullptr};
    DLabel *labelPasswd{nullptr};
    DLineEdit *editRepos{nullptr};
    DLineEdit *editLocal{nullptr};
    DLineEdit *editUser{nullptr};
    DLineEdit *editPasswd{nullptr};
    DPushButton *pbtSelLocal{nullptr};
    DPushButton *pbtOk{nullptr};
    QVBoxLayout *vLayoutPbt{nullptr};
    QGridLayout *gLayout{nullptr};
};

#endif // CHECKOUTDIALOG_H
