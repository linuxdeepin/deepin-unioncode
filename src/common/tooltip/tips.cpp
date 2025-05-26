// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tips.h"

#include <QStyle>
#include <QScreen>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QStylePainter>
#include <QGuiApplication>
#include <QStyleOptionFrame>

TipLabel::TipLabel(QWidget *parent)
    : QLabel(parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget)
{
}

TextTip::TextTip(QWidget *parent)
    : TipLabel(parent)
{
    setAutoFillBackground(true);
    setForegroundRole(QPalette::BrightText);
    setBackgroundRole(QPalette::Base);
    ensurePolished();
    setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, nullptr, this));
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft);
    setIndent(1);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, nullptr, this) / 255.0);
}

void TextTip::setContent(const QVariant &content)
{
    tipText = content.toString();
    setOpenExternalLinks(likelyContainsLink());
}

bool TextTip::isInteractive() const
{
    return likelyContainsLink();
}

void TextTip::configure(const QPoint &pos)
{
    setText(tipText);

    QFontMetrics fm(font());
    int extraHeight = 0;
    if (fm.descent() == 2 && fm.ascent() >= 11)
        ++extraHeight;

    setWordWrap(false);
    int tipWidth = sizeHint().width();

    QScreen *screen = QGuiApplication::screenAt(pos);
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    const int screenWidth = screen->availableGeometry().width();
    const int maxDesiredWidth = int(screenWidth * .5);
    if (tipWidth > maxDesiredWidth) {
        setWordWrap(true);
        tipWidth = maxDesiredWidth;
    }

    resize(tipWidth, heightForWidth(tipWidth) + extraHeight);
}

bool TextTip::canHandleContentReplacement(ContentType type) const
{
    return type == TextContent;
}

int TextTip::showTime() const
{
    return 100000 + 40 * qMax(0, tipText.size() - 100);
}

bool TextTip::equals(ContentType type, const QVariant &other) const
{
    return type == TextContent && other.canConvert<QStringList>() && other.toString() == tipText;
}

void TextTip::paintEvent(QPaintEvent *event)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.initFrom(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    QLabel::paintEvent(event);
}

void TextTip::resizeEvent(QResizeEvent *event)
{
    QStyleHintReturnMask frameMask;
    QStyleOption option;
    option.initFrom(this);
    if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
        setMask(frameMask.region);

    QLabel::resizeEvent(event);
}

bool TextTip::likelyContainsLink() const
{
    if (tipText.contains("href"), Qt::CaseInsensitive)
        return true;

    return false;
}

WidgetTip::WidgetTip(QWidget *parent)
    : TipLabel(parent)
{
    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void WidgetTip::setContent(const QVariant &content)
{
    widget = content.value<QWidget *>();
}

void WidgetTip::configure(const QPoint &pos)
{
    if (!widget || layout->count() != 0)
        return;

    move(pos);
    layout->addWidget(widget);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    adjustSize();
}

bool WidgetTip::canHandleContentReplacement(ContentType type) const
{
    Q_UNUSED(type)
    return false;
}

bool WidgetTip::equals(ContentType type, const QVariant &other) const
{
    return type == WidgetContent && other.value<QWidget *>() == widget;
}
