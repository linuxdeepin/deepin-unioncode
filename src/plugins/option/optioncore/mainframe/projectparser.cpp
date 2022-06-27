/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "projectparser.h"

#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QDebug>

static const char *TARGET_NAME = "name";
static const char *TARGET_PATH = "path";
static const char *TARGET_TARGETID = "targetID";
static const char *TARGET_BUILDCOMMAND = "buildCommand";
static const char *TARGET_BUILDARGUMENTS = "buildArguments";
static const char *TARGET_BUILDTARGET = "buildTarget";
static const char *TARGET_STOPONERROR = "stopOnError";
static const char *TARGET_USEDEFAULTCOMMAND = "useDefaultCommand";

#define PRINT_CPROJECT  0

using namespace dpfservice;
ProjectParser::ProjectParser(QObject *parent) : QObject(parent)
{

}

void ProjectParser::parse(const QString &fileName)
{
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
#if 0 //TODO(mozart):open it when correct cproject path can be got.
        QMessageBox::warning(nullptr, tr("SAX Bookmarks"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
#endif
        return;
    }

    if (read(&file)) {
        // showMessage(tr("File loaded"), 2000);
    }
}

const Targets &ProjectParser::getTargets() const
{
    return targets;
}

bool ProjectParser::read(QIODevice *device)
{
    targets.clear();

    QString errorStr;
    int errorLine = 0;
    int errorColumn = 0;

    if (!domDocument.setContent(device, true, &errorStr, &errorLine,
                                &errorColumn)) {
        QMessageBox::information(nullptr, tr("DOM Bookmarks"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return false;
    }

    auto printName = [](QDomElement ele){
        // qInfo() << ele.tagName();
    };

    QDomElement cproject = domDocument.documentElement();
    printName(cproject);
    QDomElement storageMoudle = cproject.firstChildElement();
    printName(storageMoudle);
    QDomElement cconfiguration = storageMoudle.firstChildElement("cconfiguration");
    printName(cconfiguration);
    QDomElement storageModule = cconfiguration.firstChildElement();
    printName(storageModule);
    QDomElement next = storageModule;
    printName(next);
    while (!next.isNull()) {
        if (next.hasAttribute("moduleId") && next.attribute("moduleId") == "org.eclipse.cdt.make.core.buildtargets") {
            QDomElement buildTargets = next.firstChildElement();
            printName(buildTargets);
            QDomElement target = buildTargets.firstChildElement();
            printName(target);

            while (!target.isNull()) {
                parseTargetElement(target);
                target = target.nextSiblingElement();
            }
            break;
        }
        next = next.nextSiblingElement();
    }

#if PRINT_CPROJECT
    for (auto target : targets) {
        qInfo() << "\n" << TARGET_NAME << target.name << "\n"
                << "path=" << TARGET_PATH << target.path << "\n"
                << "targetID=" << TARGET_TARGETID << target.targetID << "\n"
                << "buildCommand=" << TARGET_BUILDCOMMAND << target.buildCommand << "\n"
                << "buildArguments=" << TARGET_BUILDARGUMENTS << target.buildArguments << "\n"
                << "buildTarget=" << TARGET_BUILDTARGET << target.buildTarget << "\n"
                << "stopOnError=" << TARGET_STOPONERROR << target.stopOnError << "\n"
                << "useDefaultCommand=" << TARGET_USEDEFAULTCOMMAND << target.useDefaultCommand << "\n\n\n";
    }
#endif

    return true;
}

bool ProjectParser::write(QIODevice *device) const
{
    Q_UNUSED(device)
    return true;
}

void ProjectParser::parseTargetElement(const QDomElement &element)
{
    auto printName = [](QDomElement ele) {
        // qInfo() << ele.tagName();
    };

    auto getAttr = [](QDomElement ele, QString attr)->QString {
        QString ret;
        if (!ele.isNull() && ele.hasAttribute(attr)) {
            ret = ele.attribute(attr);
        }
        return ret;
    };

    auto getChildText = [](QDomElement ele, QString eleName)->QString {
        QString ret;

        QDomElement eleConfig = ele.firstChildElement(eleName);
        if (!eleConfig.isNull()) {
            ret = eleConfig.text();
        }
        return ret;
    };

    printName(element);

    Target target;
    target.name = getAttr(element, TARGET_NAME);
    target.path = getAttr(element, TARGET_PATH);
    target.targetID = getAttr(element, TARGET_TARGETID);

    target.buildCommand = getChildText(element, TARGET_BUILDCOMMAND);
    target.buildArguments << getChildText(element, TARGET_BUILDARGUMENTS).split(" ");
    target.buildTarget = getChildText(element, TARGET_BUILDTARGET);
    target.stopOnError = getChildText(element, TARGET_STOPONERROR) == "true" ? true : false;
    target.useDefaultCommand = getChildText(element, TARGET_USEDEFAULTCOMMAND) == "true" ? true : false;

    targets.push_back(target);
}
