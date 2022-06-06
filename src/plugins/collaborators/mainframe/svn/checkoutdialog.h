/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
