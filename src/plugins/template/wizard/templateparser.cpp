// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "templateparser.h"

#include "common/util/custompaths.h"

#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

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
    QString configPath = CustomPaths::global(CustomPaths::Templates)
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
    QString configPath = CustomPaths::global(CustomPaths::Templates)
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
    wizardInfo.wizardType = rootObject.value("wizardType").toString();

    if (wizardInfo.wizardType == "QTCtype") {
        parsePages(rootObject, wizardInfo);
        parseGenerators(rootObject, wizardInfo);
    } else {
        parseConfigures(rootObject, wizardInfo);
        parseGenerator(rootObject, wizardInfo);
    }

    return true;
}

void TemplateParser::parsePages(const QJsonObject &object, WizardInfo &wizardInfo)
{
    QJsonArray pages = object.value("pages").toArray();
    foreach (auto page , pages) {
        Page pageItem;
        pageItem.displayName = page.toObject().value("trDisplayName").toString();
        pageItem.shortTitle = page.toObject().value("trShortTitle").toString();
        pageItem.typeId = page.toObject().value("typeId").toString();

        QJsonArray datas = page.toObject().value("data").toArray();
        foreach (auto data , datas) {
            EditItem item;
            item.key = data.toObject().value("name").toString();
            item.displayName = data.toObject().value("trDisplayName").toString();
            item.type = data.toObject().value("type").toString();
            QJsonObject itemData = data.toObject().value("data").toObject();
            item.data = itemData;
            
            pageItem.items.append(item);
        }

        wizardInfo.pages.push_back(pageItem);
    }
}

void TemplateParser::parseConfigures(const QJsonObject &object, WizardInfo &wizardInfo)
{
    QJsonArray configures = object.value("configures").toArray();
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
}

void TemplateParser::parseGenerator(const QJsonObject &object, WizardInfo &wizardInfo)
{
    QJsonObject generator = object.value("generator").toObject();

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
}

void TemplateParser::parseGenerators(const QJsonObject &object, WizardInfo &wizardInfo)
{
    QJsonObject generator = object.value("generators").toArray().at(0).toObject();
    
    wizardInfo.generator.templateFile = generator.value("templateFile").toString();
    QJsonArray dataArray = generator.value("data").toArray();
    foreach (auto data, dataArray) {
        FileOperator op;
        op.sourceFile = data.toObject().value("source").toString();
        op.newFile = data.toObject().value("target").toString();
        QJsonArray replaceKeys = data.toObject().value("replaceKeys").toArray();
        foreach (auto replaceKey, replaceKeys) {
            op.replaceKeys.push_back(replaceKey.toString());
        }

        wizardInfo.generator.operations.push_back(op);
    }
}

} //namespace templateMgr
