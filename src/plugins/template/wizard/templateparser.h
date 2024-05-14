// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEPARSER_H
#define TEMPLATEPARSER_H

#include <QObject>
#include <QVector>
#include <QJsonObject>
#include <QMap>

namespace templateMgr {

struct TemplateDetail{
    QString name;
    QString path;
    bool leafNode;
};

struct TemplateCategory {
    QString type;
    QVector<TemplateDetail> templateVec;
};

struct Template {
    QString category;
    QVector<TemplateCategory> templateVec;
};

using TemplateVector = QVector<Template>;

struct EditItem {
    QString key;
    QString displayName;
    QString type;
    QStringList defaultValues;
    QJsonObject data;
    bool browse;
};

struct Page{
    QString displayName;
    QString shortTitle;
    QString typeId;
    
    QVector<EditItem> items;
};

struct FileOperator {
    QString sourceFile;
    QString newFile;
    QStringList replaceKeys;
};

struct FileGenerator {
    QString rootFolder;
    QString destPath;
    QString templateFile;
    QString newfileName;
    QVector<FileOperator> operations;
};

struct WizardInfo {
    QString type;
    QString kit;
    QString language;
    QString trDisplayName;
    QString trDescription;
    QString wizardType;
    QVector<EditItem> configures;
    QVector<Page> pages;
    FileGenerator generator;
};

class TemplateParser : public QObject
{
    Q_OBJECT
public:
    static bool readTemplateConfig(TemplateVector &templateVec);
    static bool readWizardConfig(const QString &projectPath, WizardInfo &wizardInfo);
    
    //for current deepin-ide`s template
    static void parseGenerator(const QJsonObject &object, WizardInfo &wizardInfo);
    static void parseConfigures(const QJsonObject &object, WizardInfo &wizardInfo);

    //for QtCreator`s template
    static void parseGenerators(const QJsonObject &object, WizardInfo &wizardInfo);
    static void parsePages(const QJsonObject &object, WizardInfo &wizardInfo);

signals:

private slots:

private:
    explicit TemplateParser(QObject *parent = nullptr);
    ~TemplateParser();

};

} //namespace templateMgr

Q_DECLARE_METATYPE(templateMgr::TemplateDetail)

#endif // TEMPLATEPARSER_H
