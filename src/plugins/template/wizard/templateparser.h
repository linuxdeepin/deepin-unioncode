// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEPARSER_H
#define TEMPLATEPARSER_H

#include <QObject>
#include <QVector>
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
    bool browse;
};

struct FileOperator {
    QString sourceFile;
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
    QVector<EditItem> configures;
    FileGenerator generator;
};

class TemplateParser : public QObject
{
    Q_OBJECT
public:
    static bool readTemplateConfig(TemplateVector &templateVec);
    static bool readWizardConfig(const QString &projectPath, WizardInfo &wizardInfo);

signals:

private slots:

private:
    explicit TemplateParser(QObject *parent = nullptr);
    ~TemplateParser();

};

} //namespace templateMgr

Q_DECLARE_METATYPE(templateMgr::TemplateDetail)

#endif // TEMPLATEPARSER_H
