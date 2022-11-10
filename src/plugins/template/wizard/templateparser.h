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
