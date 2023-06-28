// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <QDialog>


class QLabel;
class QLineEdit;
class QGridLayout;
class QVBoxLayout;
class CheckoutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CheckoutDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void checkoutRepos(const QString &remote, const QString &local,
                       const QString &user, const QString &passwd);

private:
    QLabel *labelRepos{nullptr};
    QLabel *labelLocal{nullptr};
    QLabel *labelUser{nullptr};
    QLabel *labelPasswd{nullptr};
    QLineEdit *editRepos{nullptr};
    QLineEdit *editLocal{nullptr};
    QLineEdit *editUser{nullptr};
    QLineEdit *editPasswd{nullptr};
    QPushButton *pbtSelLocal{nullptr};
    QPushButton *pbtOk{nullptr};
    QVBoxLayout *vLayoutPbt{nullptr};
    QGridLayout *gLayout{nullptr};
};

#endif // CHECKOUTDIALOG_H
