/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#ifndef LOGGINDIALOG_H
#define LOGGINDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

class LogginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LogginDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setName(const QString &name);
    QString name() const;
    void setPasswd(const QString &name);
    QString passwd() const;
    void setTitleText(const QString &name);
    QString titleText() const;

private:
    QLabel *titleLabel{nullptr};
    QLineEdit *nameEdit{nullptr};
    QLineEdit *passwdEdit{nullptr};
    QPushButton *pbtOk{nullptr};

Q_SIGNALS:
    void logginOk();
};

#endif // LOGGINDIALOG_H
