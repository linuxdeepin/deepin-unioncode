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

static const char *kCodeCompletion = "codeCompletion";
static const char *kModel = "model";
static const char *kGlobalLanguage = "globalLanguage";
static const char *kCommitsLanguage = "commitsLanguage";

class DetailWidgetPrivate
{
    friend class DetailWidget;

    DCheckBox *cbCodeCompletion = nullptr;
    DComboBox *globalLanguageBox = nullptr;
    DComboBox *commitsLanguageBox = nullptr;
    DComboBox *modelBox = nullptr;
};

DetailWidget::DetailWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new DetailWidgetPrivate())
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

    QHBoxLayout *completionLayout = new QHBoxLayout;
    DLabel *completionLabel = new DLabel(QLabel::tr("Code Completion:"), this);
    d->cbCodeCompletion = new DCheckBox(this);
    completionLayout->addWidget(completionLabel);
    completionLayout->addWidget(d->cbCodeCompletion);

    QHBoxLayout *languageLayout = new QHBoxLayout;
    DLabel *languageLabel = new DLabel(QLabel::tr("Global Language Preference:"), this);
    d->globalLanguageBox = new DComboBox(this);
    d->globalLanguageBox->addItem("English", CodeGeeX::En);
    d->globalLanguageBox->addItem("简体中文", CodeGeeX::Zh);
    languageLayout->addWidget(languageLabel);
    languageLayout->addWidget(d->globalLanguageBox);

    QHBoxLayout *commitsLanguageLayout = new QHBoxLayout;
    DLabel *commitsLabel = new DLabel(QLabel::tr("Commits Language Preference:"), this);
    d->commitsLanguageBox = new DComboBox(this);
    d->commitsLanguageBox->addItem("English", CodeGeeX::En);
    d->commitsLanguageBox->addItem("简体中文", CodeGeeX::Zh);
    commitsLanguageLayout->addWidget(commitsLabel);
    commitsLanguageLayout->addWidget(d->commitsLanguageBox);

    QHBoxLayout *modelLayout = new QHBoxLayout;
    DLabel *modelLabel = new DLabel(QLabel::tr("model"), this);
    d->modelBox = new DComboBox(this);
    d->modelBox->addItem("CodeGeex Lite", CodeGeeX::Lite);
    d->modelBox->addItem("CodeGeex Pro", CodeGeeX::Pro);
    modelLayout->addWidget(modelLabel);
    modelLayout->addWidget(d->modelBox);

    connect(d->modelBox, &DComboBox::currentTextChanged, this, [=](const QString &text){
        if (text == "CodeGeex Lite")
            d->modelBox->setToolTip(tr("Quickly Answer Questions"));
        else if (text == "CodeGeex Pro")
            d->modelBox->setToolTip(tr("Provide More Accurate Answers to Technical Questions"));
    });

    vLayout->addLayout(completionLayout);
    vLayout->addLayout(languageLayout);
    vLayout->addLayout(commitsLanguageLayout);
    vLayout->addLayout(modelLayout);
    vLayout->addStretch();
}

bool DetailWidget::getControlValue(QMap<QString, QVariant> &map)
{
    CodeGeeXConfig config;
    config.codeCompletionEnabled = d->cbCodeCompletion->isChecked();
    config.model = d->modelBox->currentData().value<CodeGeeX::languageModel>();
    config.globalLanguage = d->globalLanguageBox->currentData().value<CodeGeeX::locale>();
    config.commitsLanguage = d->commitsLanguageBox->currentData().value<CodeGeeX::locale>();
    dataToMap(config, map);

    Copilot::instance()->setGenerateCodeEnabled(config.codeCompletionEnabled);
    Copilot::instance()->setCommitsLocale(config.commitsLanguage == CodeGeeX::Zh ? "zh" : "cn");
    CodeGeeXManager::instance()->setLocale(config.globalLanguage);
    CodeGeeXManager::instance()->setCurrentModel(config.model);
    return true;
}

void DetailWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    // TODO(MOZART): change other value to save.
    CodeGeeXConfig config;
    mapToData(map, config);

    d->cbCodeCompletion->setChecked(config.codeCompletionEnabled);
    for (auto index = 0; index < d->modelBox->count(); index++) {
        if (d->modelBox->itemData(index) == config.model)
            d->modelBox->setCurrentIndex(index);
    }

    d->globalLanguageBox->setCurrentText(config.globalLanguage == CodeGeeX::Zh ? "简体中文" : "English");
    d->commitsLanguageBox->setCurrentText(config.commitsLanguage == CodeGeeX::Zh ? "简体中文" : "English");
}

bool DetailWidget::dataToMap(const CodeGeeXConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> apiKey;
    apiKey.insert(kCodeCompletion, config.codeCompletionEnabled);
    apiKey.insert(kModel, config.model);
    apiKey.insert(kGlobalLanguage, config.globalLanguage);
    apiKey.insert(kCommitsLanguage, config.commitsLanguage);

    map.insert("Detail", apiKey);

    return true;
}

bool DetailWidget::mapToData(const QMap<QString, QVariant> &map, CodeGeeXConfig &config)
{
    QMap<QString, QVariant> detail = map.value("Detail").toMap();
    auto var = detail.value(kCodeCompletion);
    if (var.isValid())
        config.codeCompletionEnabled = var.toBool();
    var = detail.value(kModel);
    if (var.isValid())
        config.model = var.value<CodeGeeX::languageModel>();
    var = detail.value(kGlobalLanguage);
    if (var.isValid())
        config.globalLanguage = var.value<CodeGeeX::locale>();
    var = detail.value(kCommitsLanguage);
    if (var.isValid())
        config.commitsLanguage = var.value<CodeGeeX::locale>();


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
