// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLAPSEWIDGET_H
#define COLLAPSEWIDGET_H

#include <DWidget>

#include <QPaintEvent>

DWIDGET_USE_NAMESPACE

class CollapseWidgetPrivate;
class CollapseWidget : public DWidget
{
    CollapseWidgetPrivate *const d;
public:
    explicit CollapseWidget(QWidget *parent = nullptr);
    explicit CollapseWidget(const QString &title, QWidget *widget, QWidget *parent = nullptr);
    virtual ~CollapseWidget() override;

    DWidget *takeWidget();
    void setWidget(DWidget *widget);
    DWidget* widget();

    void setTitle(const QString &title);
    QString title();

    void setCheckable(bool checkable);
    bool isCheckable();

    bool isChecked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void setChecked(bool);

private slots:
    void doChecked(bool);
    void resetHeight(const QVariant &);
};

#endif // COLLAPSEWIDGET_H
