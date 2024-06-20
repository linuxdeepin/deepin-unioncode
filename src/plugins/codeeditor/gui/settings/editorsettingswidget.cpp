// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorsettingswidget.h"
#include "fontcolorwidget.h"
#include "settingsdefine.h"

#include "services/option/optionutils.h"
#include "services/option/optionmanager.h"

#include <QTabWidget>
#include <QHBoxLayout>

QWidget *EditorSettingsWidgetGenerator::optionWidget()
{
    return new EditorSettingsWidget;
}

class EditorSettingsWidgetPrivate
{
public:
    explicit EditorSettingsWidgetPrivate(EditorSettingsWidget *qq);
    void initUI();

    EditorSettingsWidget *q;
    QTabWidget *tabWidget { nullptr };
};

EditorSettingsWidgetPrivate::EditorSettingsWidgetPrivate(EditorSettingsWidget *qq)
    : q(qq)
{
}

void EditorSettingsWidgetPrivate::initUI()
{
    tabWidget = new QTabWidget(q);
    tabWidget->tabBar()->setAutoHide(true);
    tabWidget->setDocumentMode(true);
    tabWidget->addTab(new FontColorWidget(q), Node::FontColor);

    QHBoxLayout *layout = new QHBoxLayout(q);
    layout->addWidget(tabWidget);
}

EditorSettingsWidget::EditorSettingsWidget(QWidget *parent)
    : PageWidget(parent),
      d(new EditorSettingsWidgetPrivate(this))
{
    d->initUI();
}

EditorSettingsWidget::~EditorSettingsWidget()
{
    delete d;
}

void EditorSettingsWidget::saveConfig()
{
    for (int i = 0; i < d->tabWidget->count(); ++i) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(i));
        if (pageWidget) {
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(),
                                          EditorConfig, itemNode, map);

            OptionManager::getInstance()->updateData();
        }
    }
}

void EditorSettingsWidget::readConfig()
{
    for (int i = 0; i < d->tabWidget->count(); ++i) {
        PageWidget *pageWidget = qobject_cast<PageWidget *>(d->tabWidget->widget(i));
        if (pageWidget)
            pageWidget->setUserConfig({});
    }
}
