// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGGINDIALOG_H
#define LOGGINDIALOG_H

#include <DDialog>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
class LogginDialog : public DDialog
{
    Q_OBJECT
public:
    explicit LogginDialog(QWidget *parent = nullptr);
    void setName(const QString &name);
    QString name() const;
    void setPasswd(const QString &name);
    QString passwd() const;
    void setTitleText(const QString &name);
    QString titleText() const;

private:
    DLabel *titleLabel{nullptr};
    DLineEdit *nameEdit{nullptr};
    DLineEdit *passwdEdit{nullptr};
    DPushButton *pbtOk{nullptr};

Q_SIGNALS:
    void logginOk();
};

#endif // LOGGINDIALOG_H
