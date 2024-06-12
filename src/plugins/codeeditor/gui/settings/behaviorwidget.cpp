// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "behaviorwidget.h"
#include "editorsettings.h"
#include "settingsdefine.h"

#include <DSpinBox>

#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class BehaviorWidgetPrivate
{
public:
    explicit BehaviorWidgetPrivate(BehaviorWidget *qq);

    void initUI();
    QWidget *createItem(const QString &name, QWidget *widget);

public:
    BehaviorWidget *q;

    QStringList policyList;
    QComboBox *tabPolicyCB { nullptr };
    DSpinBox *tabSizeSB { nullptr };
    DSpinBox *tipActiveTime { nullptr };
    QCheckBox *autoIndentCB { nullptr };
};

BehaviorWidgetPrivate::BehaviorWidgetPrivate(BehaviorWidget *qq)
    : q(qq)
{
    policyList << BehaviorWidget::tr("Spaces Only")
               << BehaviorWidget::tr("Tabs Only");
}

void BehaviorWidgetPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel(BehaviorWidget::tr("Tabs And Indentation"), q);

    tabPolicyCB = new QComboBox(q);
    tabPolicyCB->addItems(policyList);

    tabSizeSB = new DSpinBox(q);
    tabSizeSB->setRange(1, 20);

    autoIndentCB = new QCheckBox(BehaviorWidget::tr("Enable automatic indentation"), q);

    QHBoxLayout *itemLayout = new QHBoxLayout;
    itemLayout->setSpacing(15);
    itemLayout->addWidget(createItem(BehaviorWidget::tr("Tab policy:"), tabPolicyCB));
    itemLayout->addWidget(createItem(BehaviorWidget::tr("Tab size:"), tabSizeSB));
    itemLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

    QLabel *tipConfigLabel = new QLabel(BehaviorWidget::tr("Editor Tip"), q);
    QHBoxLayout *tipLayout = new QHBoxLayout;
    tipLayout->setSpacing(15);
    tipActiveTime = new DSpinBox(q);
    tipActiveTime->setRange(0, 2000);
    tipLayout->addWidget(createItem(BehaviorWidget::tr("Tip Active Time(ms):"), tipActiveTime));
    tipLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(itemLayout);
    mainLayout->addWidget(autoIndentCB);
    mainLayout->addWidget(tipConfigLabel);
    mainLayout->addLayout(tipLayout);
}

QWidget *BehaviorWidgetPrivate::createItem(const QString &name, QWidget *widget)
{
    QWidget *box = new QWidget(q);
    QHBoxLayout *layout = new QHBoxLayout(box);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(name, q);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(label);
    layout->addWidget(widget);
    return box;
}

BehaviorWidget::BehaviorWidget(QWidget *parent)
    : PageWidget(parent),
      d(new BehaviorWidgetPrivate(this))
{
    d->initUI();
}

BehaviorWidget::~BehaviorWidget()
{
    delete d;
}

void BehaviorWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    Q_UNUSED(map)

    auto tabPolicy = EditorSettings::instance()->value(Node::Behavior, Group::TabGroup, Key::TabPolicy, 0).toInt();
    auto tabSize = EditorSettings::instance()->value(Node::Behavior, Group::TabGroup, Key::TabSize, 4).toInt();
    auto enableIndentation = EditorSettings::instance()->value(Node::Behavior, Group::TabGroup, Key::EnableAutoIndentation, true).toBool();
    auto tipActiveTime = EditorSettings::instance()->value(Node::Behavior, Group::TipGroup, Key::TipActiveTime, 500).toInt();

    if (tabSize > d->tabSizeSB->maximum())
        tabSize = d->tabSizeSB->maximum();
    else if (tabSize < d->tabSizeSB->minimum())
        tabSize = d->tabSizeSB->minimum();
    if (tipActiveTime > d->tipActiveTime->maximum())
        tipActiveTime = d->tipActiveTime->maximum();
    else if (tipActiveTime < d->tipActiveTime->minimum())
        tipActiveTime = d->tipActiveTime->minimum();

    d->tabPolicyCB->setCurrentIndex(tabPolicy);
    d->tabSizeSB->setValue(tabSize);
    d->autoIndentCB->setChecked(enableIndentation);
    d->tipActiveTime->setValue(tipActiveTime);
}

void BehaviorWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    QVariantMap fontMap;
    fontMap.insert(Key::TabPolicy, d->tabPolicyCB->currentIndex());
    fontMap.insert(Key::TabSize, d->tabSizeSB->value());
    fontMap.insert(Key::EnableAutoIndentation, d->autoIndentCB->isChecked());
    fontMap.insert(Key::TipActiveTime, d->tipActiveTime->value());

    map.insert(Group::TabGroup, fontMap);

    EditorSettings::instance()->setValue(Node::Behavior, Group::TabGroup, Key::TabPolicy,d->tabPolicyCB->currentIndex());
    EditorSettings::instance()->setValue(Node::Behavior, Group::TabGroup, Key::TabSize, d->tabSizeSB->value());
    EditorSettings::instance()->setValue(Node::Behavior, Group::TabGroup, Key::EnableAutoIndentation, d->autoIndentCB->isChecked());
    EditorSettings::instance()->setValue(Node::Behavior, Group::TipGroup, Key::TipActiveTime, d->tipActiveTime->value());
}
