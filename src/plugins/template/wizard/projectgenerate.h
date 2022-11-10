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
