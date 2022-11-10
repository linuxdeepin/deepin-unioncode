/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "templateparser.h"

#include "common/util/custompaths.h"

#include <QFile>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

namespace templateMgr {

TemplateParser::TemplateParser(QObject *parent)
    : QObject(parent)
{

}

TemplateParser::~TemplateParser()
{

}

bool TemplateParser::readTemplateConfig(TemplateVector &templateVec)
{
    QString configPath = CustomPaths::user(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("templates")
            + QDir::separator() + QString("templates.json");

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        return false;
    }

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();

    foreach (auto key, rootObject.keys()) {
        Template tpl;
        tpl.category = key;

        QJsonArray categoryArray = rootObject.value(key).toArray();
        foreach (auto category, categoryArray) {
            TemplateCategory tplCategory;
            QJsonObject objcategory = category.toObject();
            tplCategory.type = objcategory.value("type").toString();
            QJsonArray tplDetailArray = objcategory.value("templates").toArray();

            foreach (auto detail, tplDetailArray) {
                TemplateDetail tplDetail;
                QJsonObject detailObj = detail.toObject();
                tplDetail.name = detailObj.value("name").toString();
                tplDetail.path = detailObj.value("path").toString();
                tplDetail.leafNode = detailObj.value("leafNode").toBool();

                tplCategory.templateVec.push_back(tplDetail);
            }

            tpl.templateVec.push_back(tplCategory);
        }

        templateVec.append(tpl);
    }

    return true;
}

bool TemplateParser::readWizardConfig(const QString &projectPath, WizardInfo &wizardInfo)
{
    QString configPath = CustomPaths::user(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("templates")
            + QDir::separator() + projectPath
            + QDir::separator() + QString("wizard.json");

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        return false;
    }

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();

    wizardInfo.type = rootObject.value("type").toString();
    wizardInfo.kit = rootObject.value("kit").toString();
    wizardInfo.language = rootObject.value("language").toString();
    wizardInfo.trDisplayName = rootObject.value("trDisplayName").toString();
    wizardInfo.trDescription = rootObject.value("trDescription").toString();
    QJsonArray configures = rootObject.value("configures").toArray();
    QJsonObject generator = rootObject.value("generator").toObject();

    foreach (auto configure, configures) {
        EditItem editItem;
        editItem.key = configure.toObject().value("key").toString();
        editItem.displayName = configure.toObject().value("displayName").toString();
        editItem.type = configure.toObject().value("type").toString();
        QJsonArray defaultValueArray = configure.toObject().value("defaultValues").toArray();
        foreach (auto defaultValue, defaultValueArray) {
            editItem.defaultValues.push_back(defaultValue.toString());
        }
        editItem.browse = configure.toObject().value("browse").toBool();

        wizardInfo.configures.push_back(editItem);
    }

    wizardInfo.generator.rootFolder = generator.value("rootFolder").toString();
    wizardInfo.generator.templateFile = generator.value("templateFile").toString();
    wizardInfo.generator.destPath = generator.value("destPath").toString();
    wizardInfo.generator.newfileName = generator.value("newfileName").toString();
    QJsonArray dataArray = generator.value("operations").toArray();
    foreach (auto data, dataArray) {
        FileOperator op;
        op.sourceFile = data.toObject().value("sourceFile").toString();
        QJsonArray replaceKeys = data.toObject().value("replaceKeys").toArray();
        foreach (auto replaceKey, replaceKeys) {
            op.replaceKeys.push_back(replaceKey.toString());
        }

        wizardInfo.generator.operations.push_back(op);
    }

    return true;
}

} //namespace templateMgr
