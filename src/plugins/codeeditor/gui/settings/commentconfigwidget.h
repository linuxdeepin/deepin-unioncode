// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMENTCONFIGWIDGET_H
#define COMMENTCONFIGWIDGET_H

#include "common/widget/pagewidget.h"

class CommentConfigWidgetPrivate;
class CommentConfigWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit CommentConfigWidget(QWidget *parent = nullptr);
    ~CommentConfigWidget() override;

    void initUI();
    void initConnections();

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

public slots:

private:
    CommentConfigWidgetPrivate *const d;
};

#endif // COMMENTCONFIGWIDGET_H
