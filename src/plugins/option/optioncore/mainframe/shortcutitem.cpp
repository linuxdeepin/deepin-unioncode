// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutitem.h"

#include <DStyle>
#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QPainter>
#include <QPaintEvent>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

KeyLabel::KeyLabel(QWidget *parent)
    : DFrame(parent)
{
    label = new DLabel(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(layout->contentsMargins().left(), 0, layout->contentsMargins().right(), 0);
    layout->setSpacing(0);
    label->setForegroundRole(QPalette::ButtonText);
    layout->addWidget(label);
}

void KeyLabel::setKeySquece(const QString &keySequece)
{
    label->setText(keySequece);
    update();
}

void KeyLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    QPainter p(this);
    drawShadow(&p, event->rect() - contentsMargins(), QColor(0, 0, 0, 20));

    opt.features |= QStyleOptionFrame::Rounded;

#ifdef DTKWIDGET_CLASS_DPaletteHelper
    const DPalette &dp = DPaletteHelper::instance()->palette(this);
#else
    const DPalette &dp = DGuiApplicationHelper::instance()->applicationPalette();
#endif

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        p.setBackground(QColor(255, 255, 255));
    } else {
        QColor bgColor(109, 109, 109);
        if ((opt.state & QStyle::State_Active) == 0) {
            auto inactive_mask_color = dp.color(QPalette::Window);
            inactive_mask_color.setAlphaF(0.6);
            bgColor = DGuiApplicationHelper::blendColor(bgColor, inactive_mask_color);
        }
        p.setBackground(bgColor);
    }

    p.setPen(QPen(dp.frameBorder(), opt.lineWidth));
    style()->drawControl(QStyle::CE_ShapedFrame, &opt, &p, this);
}

void KeyLabel::drawShadow(QPainter *p, const QRect &rect, const QColor &color) const
{
    DStyle dstyle;
    int frame_radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
    int shadow_xoffset = dstyle.pixelMetric(DStyle::PM_ShadowHOffset);
    int shadow_yoffset = dstyle.pixelMetric(DStyle::PM_ShadowVOffset);

    QRect shadow = rect;
    QPoint pointOffset(rect.center().x() + shadow_xoffset, rect.center().y() + shadow_yoffset);
    shadow.moveCenter(pointOffset);

    p->setBrush(color);
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing);
    p->drawRoundedRect(shadow, frame_radius, frame_radius);
}

ShortcutLabel::ShortcutLabel(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
}

void ShortcutLabel::setKeySqueces(const QStringList &keySequeces)
{
    for (int i = 0; i < keySequeces.size(); ++i) {
        auto keyLabel = new KeyLabel(this);
        keyLabel->setKeySquece(keySequeces[i]);
        layout()->addWidget(keyLabel);
    }

    qobject_cast<QHBoxLayout *>(layout())->addStretch(1);
}
