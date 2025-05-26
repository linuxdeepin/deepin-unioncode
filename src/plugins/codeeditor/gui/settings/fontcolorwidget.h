// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTCOLORWIDGET_H
#define FONTCOLORWIDGET_H

#include "common/widget/pagewidget.h"

class FontColorWidgetPrivate;
class FontColorWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit FontColorWidget(QWidget *parent = nullptr);
    ~FontColorWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

public slots:
    void fontSelected(const QFont &font);
    void fontSizeSelected(int index);

private:
    FontColorWidgetPrivate *const d;
};

#endif // FONTCOLORWIDGET_H
