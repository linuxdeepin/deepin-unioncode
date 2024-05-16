// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorsettingswidget.h"
#include "fontcolorwidget.h"
#include "behaviorwidget.h"
#include "commentconfigwidget.h"
#include "settingsdefine.h"

#include "services/option/optionutils.h"
#include "services/option/optionmanager.h"

#include <QVBoxLayout>

QWidget *EditorSettingsWidgetGenerator::optionWidget()
{
    return new EditorSettingsWidget;
}

class EditorSettingsWidgetPrivate
{
public:
    explicit EditorSettingsWidgetPrivate(EditorSettingsWidget *qq);

    void initUI();
    void saveConfig(PageWidget *page, const QString &node);
    void readConfig(PageWidget *page);

    EditorSettingsWidget *q;
    FontColorWidget *fontColorWidget { nullptr };
    BehaviorWidget *behaviorWidget { nullptr };
    CommentConfigWidget *commentConfigWidget { nullptr };
};

EditorSettingsWidgetPrivate::EditorSettingsWidgetPrivate(EditorSettingsWidget *qq)
    : q(qq)
{
}

void EditorSettingsWidgetPrivate::initUI()
{
    auto addHorizontalLine = [this](QLayout *layout) {
        QFrame *hLine = new QFrame(q);
        hLine->setFrameShape(QFrame::HLine);
        layout->addWidget(hLine);
    };

    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    fontColorWidget = new FontColorWidget(q);
    behaviorWidget = new BehaviorWidget(q);
    commentConfigWidget = new CommentConfigWidget(q);

    layout->addWidget(fontColorWidget);
    addHorizontalLine(layout);
    layout->addWidget(behaviorWidget);
    addHorizontalLine(layout);
    layout->addWidget(commentConfigWidget);
    
    q->setLayout(layout);
}

void EditorSettingsWidgetPrivate::saveConfig(PageWidget *page, const QString &node)
{
    QMap<QString, QVariant> map;
    page->getUserConfig(map);
    OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(), EditorConfig, node, map);
}

void EditorSettingsWidgetPrivate::readConfig(PageWidget *page)
{
    page->setUserConfig({});
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
    d->saveConfig(d->fontColorWidget, Node::FontColor);
    d->saveConfig(d->behaviorWidget, Node::Behavior);
    d->saveConfig(d->commentConfigWidget, Node::MimeTypeConfig);
    OptionManager::getInstance()->updateData();
}

void EditorSettingsWidget::readConfig()
{
    d->readConfig(d->fontColorWidget);
    d->readConfig(d->behaviorWidget);
    d->readConfig(d->commentConfigWidget);
}
