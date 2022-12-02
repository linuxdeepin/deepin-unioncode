/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "profilesettingwidget.h"
#include "common/common.h"

#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>

class ProfileSettingWidgetPrivate
{
    friend class ProfileSettingWidget;
    QVBoxLayout *vLayout = nullptr;
    QHBoxLayout *hlayout = nullptr;
    QLabel *languageEdit = nullptr;
    QComboBox *cbChooseLanguage = nullptr;
    LanguagePaths languagePaths;
};

ProfileSettingWidget::ProfileSettingWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new ProfileSettingWidgetPrivate)
{
    readTranslate();
    setupUi();
    readConfig();
}

ProfileSettingWidget::~ProfileSettingWidget()
{
    if(d)
        delete d;
}

QString ProfileSettingWidget::translateFilePath()
{
    return CustomPaths::global(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("translate.support");
}

QString ProfileSettingWidget::languageFilePath()
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
    QFile file(languageFilePath());
    QTextStream txtInput(&file);
    QString chooseFileName = d->languagePaths.value(d->cbChooseLanguage->currentText());
    QString currentFileName;
    if (file.open(QIODevice::ReadOnly)) {
        currentFileName = txtInput.readLine();
        file.close();
    }
    if (chooseFileName == currentFileName) {
        return;
    }

    if (file.open(QFile::WriteOnly)) {

        file.write(chooseFileName.toUtf8());
        file.close();
    }
    QMessageBox::information(this, tr("Restart Required--deep-in unioncode"),
                             tr("The language change will take effect after restart."),
                             QMessageBox::Ok);
}

void ProfileSettingWidget::readConfig()
{
    QFile file(languageFilePath());
    QTextStream txtInput(&file);
    QString fileName;
    if (file.open(QIODevice::ReadOnly)) {
        fileName = txtInput.readLine();
        file.close();
    }
    d->cbChooseLanguage->setCurrentIndex(0);
    for (int i = 0; i < d->cbChooseLanguage->count(); i++) {
        if (d->languagePaths.value(d->cbChooseLanguage->itemText(i)) == fileName) {
            d->cbChooseLanguage->setCurrentIndex(i);
            break;
        }
    }
}

void ProfileSettingWidget::setupUi()
{
    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->hlayout)
        d->hlayout = new QHBoxLayout();

    if (!d->languageEdit) {
        d->languageEdit = new QLabel(tr("language:"));
    }

    if (!d->cbChooseLanguage)
        d->cbChooseLanguage = new QComboBox();
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
    d->hlayout->addWidget(d->cbChooseLanguage, 5, Qt::AlignmentFlag::AlignLeft);

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
