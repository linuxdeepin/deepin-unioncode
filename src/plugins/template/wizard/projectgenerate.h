// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTGENERATE_H
#define PROJECTGENERATE_H

#include "templateparser.h"

#include <QObject>
#include <QVector>
#include <QMap>

namespace templateMgr {

using SettingParamMap = QMap<QString, QString>;

enum GenType {
    Unknown = 0,
    Project,
    File
};

struct PojectGenParam {
    GenType type = Unknown;
    QString kit;
    QString language;
    QString templatePath;
    SettingParamMap settingParamMap;
    FileGenerator generator;
};

struct PojectGenResult {
    QString message;
    QString kit;
    QString language;
    QString projectPath;
    QString filePath;
};

class ProjectGeneratePrivate;
class ProjectGenerate : public QObject
{
    Q_OBJECT
public:
    explicit ProjectGenerate(QObject *parent = nullptr);
    ~ProjectGenerate();

    bool create(PojectGenResult &retResult, const PojectGenParam &genParam);
signals:

private slots:

private:
    bool genProject(PojectGenResult &retResult, const PojectGenParam &genParam);
    bool copyDir(QString &retMsg, const QString &srcPath, const QString &dstPath, bool cover);
    bool transform(QString &retMsg, const PojectGenParam &genParam, const QString &projectPath);

    bool genFile(PojectGenResult &retResult, const PojectGenParam &genParam);
    bool copyFile(QString &retMsg, const QString &srcPath, const QString &dstPath, bool cover);

    ProjectGeneratePrivate *const d;
};

} //namespace templateMgr

#endif // PROJECTGENERATE_H
