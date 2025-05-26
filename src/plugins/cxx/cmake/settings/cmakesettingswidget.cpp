// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakesettingswidget.h"
#include "kitoptionwidget.h"
#include "compileroptionwidget.h"
#include "debuggeroptionwidget.h"
#include "tooloptionwidget.h"

#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <DButtonBox>

#include <QStackedWidget>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

QWidget *CMakeSettingsWidgetGenerator::optionWidget()
{
    return new CMakeSettingsWidget;
}

class CMakeSettingsWidgetPrivate : public QObject
{
public:
    explicit CMakeSettingsWidgetPrivate(CMakeSettingsWidget *qq);

    void initUI();
    void initConnection();
    void initOptions();

    void setButtonList();
    void handleSwitchWidget(QAbstractButton *btn);

public:
    CMakeSettingsWidget *q;

    DButtonBox *buttonBox { nullptr };
    QStackedWidget *stackedWidget { nullptr };

    KitOptionWidget *kitOptWidget { nullptr };
    CompilerOptionWidget *compilerOptWidget { nullptr };
    DebuggerOptionWidget *debuggerOptWidget { nullptr };
    ToolOptionWidget *toolOptWidget { nullptr };
};

CMakeSettingsWidgetPrivate::CMakeSettingsWidgetPrivate(CMakeSettingsWidget *qq)
    : q(qq)
{
}

void CMakeSettingsWidgetPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(15);

    buttonBox = new DButtonBox(q);
    stackedWidget = new QStackedWidget(q);

    mainLayout->addWidget(buttonBox, 0, Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->addWidget(stackedWidget, 1);

    initOptions();
}

void CMakeSettingsWidgetPrivate::initConnection()
{
    connect(buttonBox, &DButtonBox::buttonClicked, this, &CMakeSettingsWidgetPrivate::handleSwitchWidget);
}

void CMakeSettingsWidgetPrivate::initOptions()
{
    // add option widget
    kitOptWidget = new KitOptionWidget(q);
    compilerOptWidget = new CompilerOptionWidget(q);
    debuggerOptWidget = new DebuggerOptionWidget(q);
    toolOptWidget = new ToolOptionWidget(q);

    stackedWidget->addWidget(kitOptWidget);
    stackedWidget->addWidget(compilerOptWidget);
    stackedWidget->addWidget(debuggerOptWidget);
    stackedWidget->addWidget(toolOptWidget);

    // add option button
    setButtonList();
}

void CMakeSettingsWidgetPrivate::setButtonList()
{
    QList<DButtonBoxButton *> btnList;
    for (int i = 0; i < stackedWidget->count(); ++i) {
        auto option = dynamic_cast<BaseOption *>(stackedWidget->widget(i));
        Q_ASSERT(option);

        DButtonBoxButton *btn = new DButtonBoxButton(option->titleName(), q);
        btn->setProperty(OptionID, i);
        btnList << btn;
    }

    if (!btnList.isEmpty()) {
        buttonBox->setButtonList(btnList, true);
        btnList.first()->setChecked(true);
    }
}

void CMakeSettingsWidgetPrivate::handleSwitchWidget(QAbstractButton *btn)
{
    auto id = btn->property(OptionID).toInt();
    stackedWidget->setCurrentIndex(id);
}

CMakeSettingsWidget::CMakeSettingsWidget(QWidget *parent)
    : PageWidget(parent),
      d(new CMakeSettingsWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

CMakeSettingsWidget::~CMakeSettingsWidget()
{
    delete d;
}

void CMakeSettingsWidget::saveConfig()
{
    for (int i = 0; i < d->stackedWidget->count(); ++i) {
        auto option = dynamic_cast<BaseOption *>(d->stackedWidget->widget(i));
        Q_ASSERT(option);

        const auto &map = option->getConfig();
        OptionManager::getInstance()->setValue(option::CATEGORY_CMAKE, option->configName(), map);
    }
}

void CMakeSettingsWidget::readConfig()
{
    // Finally set kit configuration, make sure
    // that all other configuration has been configured
    for (int i = d->stackedWidget->count() - 1; i >= 0; --i) {
        auto widget = d->stackedWidget->widget(i);
        auto option = dynamic_cast<BaseOption *>(widget);
        Q_ASSERT(option);

        QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(option::CATEGORY_CMAKE, option->configName()).toMap();
        option->setConfig(map);
        auto optWidget = dynamic_cast<BaseOptionWidgetHelper *>(widget);
        if (optWidget)
            d->kitOptWidget->setOptions(optWidget->getOptions());
    }
}
