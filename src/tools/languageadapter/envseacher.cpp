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
#include "envseacher.h"

QVariantHash EnvSeacher::python3()
{
    QVariantHash ret;
    // get python version
    Version version;
    ProcessUtil::execute("python3", {"-V"}, [&](const QByteArray &data){
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
            if (!major.isNull())
                version.major = major.toInt();
            if (!minor.isNull())
                version.minor = minor.toInt();
            if (!revsion.isNull())
                version.revision = revsion.toInt();
            if (!build.isNull())
                version.revision = build.toInt();
        }
        ret.insert(K_Version, QVariant::fromValue<Version>(version));
    });

    UserEnv userEnv;
    userEnv.binsPath = QDir::homePath() + QDir::separator()
            + ".local" + QDir::separator()
            + "bin";
    userEnv.pkgsPath = QDir::homePath() + QDir::separator()
            + ".local" + QDir::separator()
            + "lib" + QDir::separator()
            + QString("python")
            + QString::number(version.major.value())
            + QString::number(version.minor.value())
            + QDir::separator()
            + "site-packages";
    ret.insert(K_UserEnv, QVariant::fromValue<UserEnv>(userEnv));

    return ret;
}

QVariantHash EnvSeacher::cxx()
{
    QVariantHash ret;
    return ret;
}

QVariantHash EnvSeacher::java()
{
    QVariantHash ret;
    return ret;
}
