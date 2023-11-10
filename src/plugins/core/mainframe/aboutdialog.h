// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <DAbstractDialog>

class AboutDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);
    virtual ~AboutDialog();

signals:

private:
    void setupUi();
    bool bExpand = false;

public slots:
    void handleLinkActivated(const QString& link);
};

#endif // ABOUTDIALOG_H
