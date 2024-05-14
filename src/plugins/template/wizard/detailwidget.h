// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H


#include "projectgenerate.h"
#include "services/language/languageservice.h"

#include <DScrollArea>

DWIDGET_USE_NAMESPACE
using namespace templateMgr;

class DetailWidgetPrivate;
class DetailWidget : public DTK_WIDGET_NAMESPACE::DScrollArea
{
    Q_OBJECT
public:
    explicit DetailWidget(DWidget *parent = nullptr);
    DetailWidget(const QString &templatePath, DWidget *parent = nullptr);
    ~DetailWidget() override;

    bool getGenParams(PojectGenParam &param);
signals:
    void closeSignal();  // 声明自定义信号

private slots:

private:
    DetailWidgetPrivate *const d;
    void generate(const PojectGenParam &param);
    QString getVariable(const QString &variable, PojectGenParam &param);

};

#endif // DETAILWIDGET_H
