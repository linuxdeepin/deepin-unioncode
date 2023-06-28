// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
