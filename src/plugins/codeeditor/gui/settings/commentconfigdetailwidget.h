// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMENTCONFIGDETALWIDGET_H
#define COMMENTCONFIGDETALWIDGET_H

#include "common/widget/pagewidget.h"

class CommentConfigDetailWidgetPrivate;
class CommentConfigDetailWidget : public PageWidget
{
public:
    CommentConfigDetailWidget(QWidget *parent, int index);

    void initUI();
    void initConnections();
    ~CommentConfigDetailWidget();

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

private:
    CommentConfigDetailWidgetPrivate *const d = nullptr;
};

#endif // COMMENTCONFIGDETALWIDGET_H
