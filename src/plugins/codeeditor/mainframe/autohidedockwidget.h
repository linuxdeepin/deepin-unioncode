// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTOHIDEDOCKWIDGET_H
#define AUTOHIDEDOCKWIDGET_H

#include <DDockWidget>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class AutoHideDockWidget : public DDockWidget
{
    Q_OBJECT
public:
    AutoHideDockWidget(const QString &title, QWidget *parent = nullptr,
                       Qt::WindowFlags flags = Qt::WindowFlags());
    explicit AutoHideDockWidget(QWidget *parent = nullptr,
                                Qt::WindowFlags flags = Qt::WindowFlags());

    DWidget *widget() const;

    void setWidget(DWidget *widget);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
};

#endif // AUTOHIDEDOCKWIDGET_H
