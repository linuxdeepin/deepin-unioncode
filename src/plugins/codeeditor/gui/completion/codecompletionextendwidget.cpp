// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codecompletionextendwidget.h"
#include "gui/texteditor.h"

#include <DLabel>
#include <DFrame>
#include <DIconButton>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>

#include <QLabel>
#include <QPlainTextEdit>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class KeyLabel : public DFrame
{
public:
    KeyLabel(QString text, QWidget *parent = nullptr)
        : DFrame(parent)
    {
        label = new DLabel(text, this);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(layout->contentsMargins().left(), 0, layout->contentsMargins().right(), 0);
        layout->setSpacing(0);
        label->setForegroundRole(QPalette::ButtonText);
        layout->addWidget(label);
    }

    void setKey(const QString &key)
    {
        label->setText(key);
    }

    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        QPainter p(this);
        drawShadow(&p, event->rect() - contentsMargins(), QColor(0, 0, 0, 20));

        opt.features |= QStyleOptionFrame::Rounded;

        const DPalette &dp = DPaletteHelper::instance()->palette(this);

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

    void drawShadow(QPainter *p, const QRect &rect, const QColor &color) const
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

private:
    DLabel *label;
};

class CodeCompletionExtendWidgetPrivate
{
public:
    explicit CodeCompletionExtendWidgetPrivate(CodeCompletionExtendWidget *qq);

    void initUI();
    void execute();
    QKeySequence toKeySequence(QKeyEvent* event);

public:
    CodeCompletionExtendWidget *q;

    TextEditor *editor { nullptr };
    QKeySequence shortcutKey;
    DIconButton *iconButton { nullptr };
    KeyLabel *shortcutKeyLabel { nullptr };
    QPlainTextEdit *messageEdit { nullptr };
};

CodeCompletionExtendWidgetPrivate::CodeCompletionExtendWidgetPrivate(CodeCompletionExtendWidget *qq)
    : q(qq)
{
}

void CodeCompletionExtendWidgetPrivate::initUI()
{
    auto fromLabel = new QLabel(q->tr("From:"), q);
    iconButton = new DIconButton(q);
    iconButton->setIconSize({ 24, 24 });
    iconButton->setFlat(true);

    auto shortcutLabel = new QLabel(q->tr("Shortcut:"), q);
    shortcutKeyLabel = new KeyLabel("", q);

    messageEdit = new QPlainTextEdit(q);
    messageEdit->setReadOnly(true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(5, 0, 0, 0);
    hLayout->addWidget(fromLabel);
    hLayout->addWidget(iconButton);
    hLayout->addSpacing(15);
    hLayout->addWidget(shortcutLabel);
    hLayout->addWidget(shortcutKeyLabel);
    hLayout->addSpacerItem(new QSpacerItem(20, 1, QSizePolicy::Expanding));

    QVBoxLayout *vLayout = new QVBoxLayout(q);
    vLayout->setContentsMargins(0, 5, 0, 0);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(messageEdit, 1);
}

void CodeCompletionExtendWidgetPrivate::execute()
{
    if (!editor)
        return;

    auto completionText = messageEdit->toPlainText();
    editor->insertText(completionText);
}

QKeySequence CodeCompletionExtendWidgetPrivate::toKeySequence(QKeyEvent *event)
{
    return QKeySequence(event->key() | event->modifiers());
}

CodeCompletionExtendWidget::CodeCompletionExtendWidget(QWidget *parent)
    : QWidget(parent),
      d(new CodeCompletionExtendWidgetPrivate(this))
{
    d->initUI();
}

CodeCompletionExtendWidget::~CodeCompletionExtendWidget()
{
    delete d;
}

void CodeCompletionExtendWidget::setCompletionInfo(const QString &info, const QIcon &icon, const QKeySequence &key)
{
    d->iconButton->setIcon(icon);
    d->shortcutKeyLabel->setKey(key.toString());
    d->shortcutKey = key;
    d->messageEdit->setPlainText(info);

    emit completionChanged();
}

bool CodeCompletionExtendWidget::processEvent(QKeyEvent *event)
{
    if (d->shortcutKey.isEmpty())
        return false;

    auto ss = d->toKeySequence(event);
    auto sdd = ss.toString();
    if (d->toKeySequence(event) == d->shortcutKey) {
        d->execute();
        return true;
    }

    return false;
}

bool CodeCompletionExtendWidget::isValid()
{
    return !d->messageEdit->toPlainText().isEmpty();
}

void CodeCompletionExtendWidget::setTextEditor(TextEditor *editor)
{
    d->editor = editor;
}

void CodeCompletionExtendWidget::hideEvent(QHideEvent *event)
{
    d->messageEdit->clear();
    QWidget::hideEvent(event);
}
