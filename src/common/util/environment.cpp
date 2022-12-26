/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "config.h"
#include "environment.h"
#include "processutil.h"

#include <QRegularExpression>
#include <QDir>
#include <QDebug>
#include <QDirIterator>

#include <iostream>
namespace environment {

const QString RK_Major{"Major"};
const QString RK_Minor{"Minor"};
const QString RK_Revision{"Revision"};
const QString RK_Build{"Build"};

namespace language {

Version completion(Category category, Kit kit, const Version &version)
{
    qInfo() << __FUNCTION__ << version.major.value();
    Version ret = version;
    if (category == User) {
        if (kit == Python) {
            QString program = "python";
            if (version.major) {
                program += QString::number(version.major.value());
            }
            qInfo() << program;
            ProcessUtil::execute(program, {"-V"}, [&](const QByteArray &data){
                qInfo() << data;
                QRegularExpression regExp {"Python\\s(?<" + RK_Major +">[0-9])"
                            + "\\.(?<"+ RK_Minor +">[0-9])"
                            + "\\.(?<"+ RK_Revision +">[0-9])"
                            + "(\\.(?<"+ RK_Build + ">[0-9]))?"};
                auto matchs = regExp.match(data);
                if (matchs.hasMatch()) {
                    auto major = matchs.captured(RK_Major);
                    auto minor = matchs.captured(RK_Minor);
                    auto revsion = matchs.captured(RK_Revision);
                    auto build = matchs.captured(RK_Build);
                    if (!major.isNull()) {
                        ret.major = major.toInt();
                    }
                    if (!minor.isNull()) {
                        ret.minor = minor.toInt();
                    }
                    if (!revsion.isNull()) {
                        ret.revision = revsion.toInt();
                    }
                    if (!build.isNull()) {
                        ret.revision = build.toInt();
                    }
                }
            });
        }
    }
    return ret;
}

Program search(Category category, Kit kit, const Version &version)
{
    qInfo() << __FUNCTION__;
    Program ret;
    if (category == User) {
        if (kit == Python) {
            Version executeVersion = completion(category, kit, version);
            if (executeVersion.major.value() == 3) {
                QString program = "python";
                if (executeVersion.major) {
                    program + QString::number(executeVersion.major.value());
                }
                ret.binsPath = QDir::homePath() + QDir::separator() + ".local" + QDir::separator() + "bin";
                ret.pkgsPath = QDir::homePath() + QDir::separator()
                        + ".local" + QDir::separator()
                        + "lib" + QDir::separator()
                        + QString("python") + QString::number(executeVersion.major.value())
                        + "."
                        + QString::number(executeVersion.minor.value())
                        + QDir::separator()
                        + "site-packages";
            }
        }
    }
    return ret;
}

QProcessEnvironment get(Category category, Kit kit, const Version &version)
{
    qInfo() << __FUNCTION__;
    QProcess process;
    auto procEnv = process.processEnvironment();
    if (category == Category::User) {
        if (kit == Kit::Python) {
            Program program = search(category, kit, version);
            if (program.binsPath) {
                QString runtimeBinPath = program.binsPath.value();
                QString PATH_EnvValue = procEnv.value("PATH");
                procEnv.remove("PATH");
                procEnv.insert("PATH", runtimeBinPath + ":" + PATH_EnvValue);
            }
            if (program.pkgsPath) {
                QString userPythonPkgPath = program.pkgsPath.value();
                procEnv.remove("PYTHONPATH");
                procEnv.insert("PYTHONPATH", userPythonPkgPath);
            }
        }
    }
    return procEnv;
}

Version::Version()
{

}

Version::Version(int major)
    : major(major)
{

}

Version::Version(const Version &version)
    : major(version.major)
    , minor(version.minor)
    , revision(version.revision)
    , build(version.build)
{

}

Version &Version::operator =(const Version &version)
{
    this->major = version.major;
    this->minor = version.minor;
    this->revision = version.revision;
    this->build = version.build;
    return *this;
}

} // language

/*!
 * @brief package::native::path
 *  get local install offline packge path
 * @param category use package name to search
 * @return if Category is empty return install top path, else return package name path
 */
QString package::native::path(const Category::type_value &category)
{
    QString envPath = QString(LIBRARY_INSTALL_PREFIX)
            + QDir::separator() + "tools"
            + QDir::separator() + "env";
    QDirIterator dirItera(envPath, QDir::Files|QDir::NoDotAndDotDot);
    if (category.isEmpty()) {
        return envPath;
    } else {
        QDirIterator sameItera(envPath, QDir::Files|QDir::NoDotAndDotDot);
        while (sameItera.hasNext()) {
            sameItera.next();
            QFileInfo info = sameItera.fileInfo();
            if (category == info.fileName()) {
                return info.filePath();
            }
        }
        QDirIterator containsItera(envPath, QDir::Files|QDir::NoDotAndDotDot);
        while (containsItera.hasNext()) {
            containsItera.next();
            QFileInfo info = containsItera.fileInfo();
            if (info.fileName().contains(category)) {
                return info.filePath();
            }
        }
    }
    return envPath;
}

/*!
 * \brief environment::package::native::installed
 *  Determine whether the current offline package is installed
 * \return if install return true, else return false
 */
bool environment::package::native::installed()
{
    QDir dir(path());
    if (dir.exists())
        return true;
    return false;
}

} // environment
