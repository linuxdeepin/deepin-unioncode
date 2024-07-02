// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DETAILSBUTTON_H
#define DETAILSBUTTON_H

#include <QAbstractButton>

class DetailsButtonPrivate;
class DetailsButton : public QAbstractButton
{
    Q_OBJECT
    class DetailsButtonPrivate *const d;
public:
    explicit DetailsButton(QWidget *parent = nullptr);
    virtual ~DetailsButton() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // DETAILSBUTTON_H
