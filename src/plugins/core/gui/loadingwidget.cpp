// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadingwidget.h"

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

static float opacity = 1.0;

loadingWidget::loadingWidget(QWidget *parent)
    : DWidget(parent)
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);

    setLogo();

    loadingText = new DLabel(this);
    loadingText->setText(tr("loading..."));
    loadingText->setAlignment(Qt::AlignCenter);

    vlayout->addWidget(backgroundLogo);
    vlayout->addWidget(loadingText);
    vlayout->setAlignment(Qt::AlignCenter);
}

void loadingWidget::setLogo()
{
    backgroundLogo = new DLabel(this);
    backgroundLogo->setPixmap(QIcon::fromTheme("deepin_unioncode_backgroundLogo").pixmap(128));

    logo = new DLabel(backgroundLogo);
    logo->setPixmap(QIcon::fromTheme("deepin_unioncode_logo").pixmap(128));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(logo);
    backgroundLogo->setLayout(hlayout);

    opacityEffect = new QGraphicsOpacityEffect(this);

    connect(&timer, &QTimer::timeout, this, [=]() {
        opacity -= 0.1;

        if (opacity < -0.9)
            opacity = 1.0;

        opacityEffect->setOpacity(qAbs(opacity));
        logo->setGraphicsEffect(opacityEffect);
    });
    timer.start(150);
}
