// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "builderwidget.h"

#include <DGuiApplicationHelper>

BuilderWidget::BuilderWidget(QWidget *parent) : QWidget(parent)
{
    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType) {
        backGroundColor = QColor(25, 25, 25, 204);
    } else {
        backGroundColor = QColor(235, 235, 235, 204);
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, [&](){
        this->update();
    });
}

BuilderWidget::~BuilderWidget()
{

}

void BuilderWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType) {
        backGroundColor = QColor(25, 25, 25, 204);
    } else {
        backGroundColor = QColor(235, 235, 235, 204);
    }
    painter.fillRect(rect(), backGroundColor);
}
