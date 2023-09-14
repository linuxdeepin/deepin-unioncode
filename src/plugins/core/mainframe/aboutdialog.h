// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QDialog *parent = nullptr);
    virtual ~AboutDialog();

signals:

private:
    void setupUi();

public slots:
    void handleLinkActivated(const QString& link);
};

#endif // ABOUTDIALOG_H
