// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "common/util/custompaths.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QRadioButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>

static const char *kApiKey = "apiKey";

// this is a temporary key
static const char *kDefaultApiKey = "f30ea902c3824ee88e221a32363c0823";

class DetailWidgetPrivate
{
    friend class DetailWidget;

    QLineEdit *apiKeyWidget = nullptr;
};

DetailWidget::DetailWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new DetailWidgetPrivate())
{
    setupUi();
    updateUi();
}

DetailWidget::~DetailWidget()
{
    if (d) {
        delete d;
    }
}

void DetailWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel(QLabel::tr("CodeGeeX Api Key:"));
    d->apiKeyWidget = new QLineEdit();
    hLayout->addWidget(label);
    hLayout->addWidget(d->apiKeyWidget);

    vLayout->addLayout(hLayout);
    vLayout->addStretch();
}

void DetailWidget::updateUi()
{
}

bool DetailWidget::getControlValue(QMap<QString, QVariant> &map)
{
    CodeGeeXConfig config;
    config.apiKey = d->apiKeyWidget->text();
    dataToMap(config, map);

    return true;
}

void DetailWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    CodeGeeXConfig config;
    mapToData(map, config);

    if (config.apiKey.isEmpty()) {
        config.apiKey = kDefaultApiKey;
    }
    d->apiKeyWidget->setText(config.apiKey);
}

bool DetailWidget::dataToMap(const CodeGeeXConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> apiKey;
    apiKey.insert(kApiKey, config.apiKey);

    map.insert(tr("Detail"), apiKey);

    return true;
}

bool DetailWidget::mapToData(const QMap<QString, QVariant> &map, CodeGeeXConfig &config)
{
    QMap<QString, QVariant> detail = map.value(tr("Detail")).toMap();
    config.apiKey = detail.value(kApiKey).toString();

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
