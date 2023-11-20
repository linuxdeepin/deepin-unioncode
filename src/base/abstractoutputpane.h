// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRUCTOUTPUTPANE_H
#define ABSTRUCTOUTPUTPANE_H

#include <DWidget>

#include <QObject>

DWIDGET_USE_NAMESPACE

class AbstractOutputPane : public QObject
{
    Q_OBJECT
public:
    explicit AbstractOutputPane(QObject *parent = nullptr);
    virtual ~AbstractOutputPane() override;

    virtual DWidget *outputWidget(QWidget *parent) = 0;
    virtual void clearContents() = 0;

    virtual void setFocus() = 0;
    virtual bool hasFocus() const = 0;
    virtual bool canFocus() const = 0;

    virtual bool canNext() const = 0;
    virtual bool canPrevious() const = 0;
    virtual void goToNext() = 0;
    virtual void goToPrev() = 0;
};

#endif // ABSTRUCTOUTPUTPANE_H
