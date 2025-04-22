// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent *event) override;
#else
    void enterEvent(QEnterEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
};

#endif // DETAILSBUTTON_H
