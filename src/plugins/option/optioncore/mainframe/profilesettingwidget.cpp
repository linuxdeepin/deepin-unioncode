// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profilesettingwidget.h"
#include "common/common.h"
#include "services/option/optionmanager.h"

#include <DLabel>
#include <DComboBox>
#include <DPushButton>
#include <DDialog>

#include <QBoxLayout>

DWIDGET_USE_NAMESPACE

constexpr char kGeneralGroup[] { "General" };
constexpr char kLanguageKey[] { "Language" };
constexpr char kName[] { "name" };
constexpr char kPath[] { "path" };

class ProfileSettingWidgetPrivate
{
    friend class ProfileSettingWidget;
    QVBoxLayout *vLayout = nullptr;
    QHBoxLayout *hlayout = nullptr;
    DLabel *languageEdit = nullptr;
    DComboBox *cbChooseLanguage = nullptr;
    LanguagePaths languagePaths;
};

ProfileSettingWidget::ProfileSettingWidget(QWidget *parent)
    : PageWidget(parent), d(new ProfileSettingWidgetPrivate)
{
    readTranslate();
    setupUi();
    readConfig();
}

ProfileSettingWidget::~ProfileSettingWidget()
{
    if (d)
        delete d;
}

QString ProfileSettingWidget::translateFilePath()
{
    return CustomPaths::global(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("translate.support");
}

Q_DECL_DEPRECATED_X("-------------存在兼容代码需要删除") QString ProfileSettingWidget::languageFilePath()
{
    return CustomPaths::user(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("chooselanguage.support");
}

const LanguagePaths &ProfileSettingWidget::getLanguagePaths() const
{
    return d->languagePaths;
}

void ProfileSettingWidget::saveConfig()
{
    QVariantMap map = OptionManager::getInstance()->getValue(kGeneralGroup, kLanguageKey).toMap();
    auto languageName = d->cbChooseLanguage->currentText();
    if (map.value(kName) == languageName)
        return;

    map.insert(kName, languageName);
    map.insert(kPath, d->languagePaths.value(languageName));
    OptionManager::getInstance()->setValue(kGeneralGroup, kLanguageKey, map);

    DDialog msgBox;
    msgBox.addButton(tr("Ok"));
    msgBox.setWindowTitle(tr("Restart Required--deep-in unioncode"));
    msgBox.setMessage(tr("The language change will take effect after restart."));
    msgBox.exec();
}

void ProfileSettingWidget::readConfig()
{
    QString languageName;
    QFile file(languageFilePath());
     if (file.exists()) {
        QTextStream txtInput(&file);
        if (file.open(QIODevice::ReadOnly)) {
            languageName = d->languagePaths.key(txtInput.readLine());
            file.close();
            file.remove();
        }
        QVariantMap map;
        map.insert(kName, languageName);
        map.insert(kPath, d->languagePaths.value(languageName));

        OptionManager::getInstance()->setValue(kGeneralGroup, kLanguageKey, map);
    } else {
        const auto &map = OptionManager::getInstance()->getValue(kGeneralGroup, kLanguageKey).toMap();
        languageName = map.value(kName).toString();
    }

    d->cbChooseLanguage->setCurrentText(languageName);
}

void ProfileSettingWidget::setupUi()
{
    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->hlayout)
        d->hlayout = new QHBoxLayout();

    if (!d->languageEdit) {
        d->languageEdit = new DLabel(tr("language:"));
    }

    if (!d->cbChooseLanguage)
        d->cbChooseLanguage = new DComboBox();
    d->cbChooseLanguage->setFixedWidth(200);

    QStringList nameList = d->languagePaths.keys();
    int i = 0;
    for (auto name : nameList) {
        d->cbChooseLanguage->insertItem(i, name);
        i++;
    }

    d->hlayout->setMargin(10);
    d->hlayout->setSpacing(10);
    d->hlayout->addWidget(d->languageEdit);
    d->hlayout->addWidget(d->cbChooseLanguage, 5, Qt::AlignmentFlag::AlignRight);

    d->vLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
    d->vLayout->addLayout(d->hlayout);
}

void ProfileSettingWidget::readTranslate()
{
    QFile file(translateFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();
        for (auto node : array) {
            auto obj = node.toObject();
            QJsonValue nameVal = obj.value(lNameItem);
            QString name = nameVal.toString();

            QJsonValue pathVal = obj.value(lPathItem);
            QString path = pathVal.toString();
            d->languagePaths.insert(name, path);
        }
    }
}
