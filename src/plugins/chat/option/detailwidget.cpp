// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "common/util/custompaths.h"

#include <DCheckBox>
#include <DComboBox>
#include <DLabel>
#include <DLineEdit>

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QAction>

DWIDGET_USE_NAMESPACE

static const char *kGlobalLanguage = "globalLanguage";
static const char *kCommitsLanguage = "commitsLanguage";

class DetailWidgetPrivate
{
    friend class DetailWidget;

    DComboBox *globalLanguageBox = nullptr;
    DComboBox *commitsLanguageBox = nullptr;
};

DetailWidget::DetailWidget(QWidget *parent)
    : PageWidget(parent), d(new DetailWidgetPrivate())
{
    setupUi();
}

DetailWidget::~DetailWidget()
{
    if (d) {
        delete d;
    }
}

void DetailWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    setLayout(vLayout);

    QHBoxLayout *languageLayout = new QHBoxLayout;
    DLabel *languageLabel = new DLabel(QLabel::tr("Global Language Preference:"), this);
    d->globalLanguageBox = new DComboBox(this);
    d->globalLanguageBox->addItem("English", Chat::En);
    d->globalLanguageBox->addItem("简体中文", Chat::Zh);
    languageLayout->addWidget(languageLabel);
    languageLayout->addWidget(d->globalLanguageBox);

    QHBoxLayout *commitsLanguageLayout = new QHBoxLayout;
    DLabel *commitsLabel = new DLabel(QLabel::tr("Commits Language Preference:"), this);
    d->commitsLanguageBox = new DComboBox(this);
    d->commitsLanguageBox->addItem("English", Chat::En);
    d->commitsLanguageBox->addItem("简体中文", Chat::Zh);
    commitsLanguageLayout->addWidget(commitsLabel);
    commitsLanguageLayout->addWidget(d->commitsLanguageBox);

    vLayout->addLayout(languageLayout);
    vLayout->addLayout(commitsLanguageLayout);
    vLayout->addStretch();
}

bool DetailWidget::getControlValue(QMap<QString, QVariant> &map)
{
    ChatConfig config;
    config.globalLanguage = d->globalLanguageBox->currentData().value<Chat::Locale>();
    config.commitsLanguage = d->commitsLanguageBox->currentData().value<Chat::Locale>();
    dataToMap(config, map);

    Copilot::instance()->setCommitsLocale(config.commitsLanguage);
    ChatManager::instance()->setLocale(config.globalLanguage);
    return true;
}

void DetailWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    // TODO(MOZART): change other value to save.
    ChatConfig config;
    mapToData(map, config);

    d->globalLanguageBox->setCurrentText(config.globalLanguage == Chat::Zh ? "简体中文" : "English");
    d->commitsLanguageBox->setCurrentText(config.commitsLanguage == Chat::Zh ? "简体中文" : "English");
}

bool DetailWidget::dataToMap(const ChatConfig &config, QMap<QString, QVariant> &map)
{
    map.insert(kGlobalLanguage, config.globalLanguage);
    map.insert(kCommitsLanguage, config.commitsLanguage);

    return true;
}

bool DetailWidget::mapToData(const QMap<QString, QVariant> &map, ChatConfig &config)
{
    auto var = map.value(kGlobalLanguage);
    if (var.isValid())
        config.globalLanguage = var.value<Chat::Locale>();
    var = map.value(kCommitsLanguage);
    if (var.isValid())
        config.commitsLanguage = var.value<Chat::Locale>();

    return true;
}

void DetailWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void DetailWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}
