// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "common/util/custompaths.h"
#include "copilot.h"

#include <DCheckBox>

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QRadioButton>
#include <DLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DLabel>
#include <QHeaderView>
#include <QDebug>

DWIDGET_USE_NAMESPACE

static const char *kCodeCompletion = "codeCompletion";

class DetailWidgetPrivate
{
    friend class DetailWidget;

    DCheckBox *cbCodeCompletion = nullptr;
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

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    DLabel *label = new DLabel(DLabel::tr("Code Completion:"));
    d->cbCodeCompletion = new DCheckBox(this);
    hLayout->addWidget(label);
    hLayout->addWidget(d->cbCodeCompletion);
    connect(d->cbCodeCompletion, &DCheckBox::clicked, this, [](bool checked){
        Copilot::instance()->setGenerateCodeEnabled(checked);
    });

    vLayout->addLayout(hLayout);
    vLayout->addStretch();
}

bool DetailWidget::getControlValue(QMap<QString, QVariant> &map)
{
    CodeGeeXConfig config;
    config.codeCompletionEnabled = d->cbCodeCompletion->isChecked();
    dataToMap(config, map);

    return true;
}

void DetailWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    // TODO(MOZART): change other value to save.
    CodeGeeXConfig config;
    mapToData(map, config);

    d->cbCodeCompletion->setChecked(config.codeCompletionEnabled);

    Copilot::instance()->setGenerateCodeEnabled(config.codeCompletionEnabled);
}

bool DetailWidget::dataToMap(const CodeGeeXConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> apiKey;
    apiKey.insert(kCodeCompletion, config.codeCompletionEnabled);

    map.insert("Detail", apiKey);

    return true;
}

bool DetailWidget::mapToData(const QMap<QString, QVariant> &map, CodeGeeXConfig &config)
{
    QMap<QString, QVariant> detail = map.value("Detail").toMap();
    auto var = detail.value(kCodeCompletion);
    if (var.isValid())
        config.codeCompletionEnabled = var.toBool();

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
