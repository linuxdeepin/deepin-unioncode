// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGUREWIDGET_H
#define CONFIGUREWIDGET_H

#include <DScrollArea>

class CollapseWidget;
class ConfigureWidgetPrivate;
class ConfigureWidget : public DTK_WIDGET_NAMESPACE::DScrollArea
{
    Q_OBJECT
    ConfigureWidgetPrivate *const d;
public:
    explicit ConfigureWidget(QWidget *parent = nullptr);
    virtual ~ConfigureWidget() override;
    void addCollapseWidget(CollapseWidget *widget);
    void addWidget(DTK_WIDGET_NAMESPACE::DWidget *widget);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // CONFIGUREWIDGET_H
