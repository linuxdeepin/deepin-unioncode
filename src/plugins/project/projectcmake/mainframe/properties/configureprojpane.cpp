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
#include "configureprojpane.h"
#include "kitmanager.h"

#include "mainframe/cmakeasynparse.h"
#include "mainframe/cmakegenerator.h"

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSpacerItem>
#include <QFileDialog>
#include <QUrl>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QRadioButton>
#include <QButtonGroup>

// TODO(mozart):should get from kitmanager.
static const char *kDefaultKitName = "Desktop";
static const char *kConfigFileName = "configure.support";

class ConfigureProjPanePrivate
{
    friend class ConfigureProjPane;
    QRadioButton *cbRWithDInfo = nullptr;
    QPushButton *btnDebug = nullptr;
    QLineEdit *lineEditRWithDInfo = nullptr;
    QRadioButton *cbDebug = nullptr;
    QPushButton *btnRelease = nullptr;
    QLineEdit *lineEditRelease = nullptr;
    QRadioButton *cbMiniSize = nullptr;
    QRadioButton *cbRelease = nullptr;
    QLineEdit *lineEditDebug = nullptr;
    QLineEdit *lineEditMiniSize = nullptr;
    QPushButton *btnMinimumSize = nullptr;
    QCheckBox *cbDesktop = nullptr;
    QPushButton *btnRWithDInfo = nullptr;
    QPushButton *btnConfigure = nullptr;

    QButtonGroup *group = nullptr;

    QString projectPath;
    QString language;
    // TODO(mozart):just recored one config item now.
    QVector<ConfigureProjPane::ConfigureParam> cfgItems;
};

ConfigureProjPane::ConfigureProjPane(const QString &language,
                                     const QString &projectPath,
                                     QWidget *parent)
    : QWidget(parent)
    , d(new ConfigureProjPanePrivate)
{
    setupUi(this);
    setProjectPath(language, projectPath);
}

ConfigureProjPane::~ConfigureProjPane()
{
    if (d)
        delete d;
}

void ConfigureProjPane::setProjectPath(const QString &language, const QString &projectPath)
{
    d->projectPath = projectPath;
    d->language = language;

    // restore ui parameters from local config file.
    bool successful = restore();
    useDefaultValue();
    refreshUi();
    if (!successful) {

    } else {
        // no need show config pane again.
        updateKitInfo();

        QTimer::singleShot(0, this, [this] {
            //emit configureDone();
        });
    }
}

ConfigureProjPane::BuildType ConfigureProjPane::getDefaultBuildType() const
{
    // TODO(Mozart)
    return {};
}

QString ConfigureProjPane::getDefaultOutputPath() const
{
    QFileInfo f(d->projectPath);
    QDir dir = f.dir();
    dir.cdUp();
    QString defaultOutputPath = dir.path() + "/build-Default";
    for (auto &item : d->cfgItems) {
        if (item.kitName == kDefaultKitName) {
            if (item.checked) {
                if (item.debug.checked) {
                    defaultOutputPath = item.debug.folder;
                } else if (item.release.checked) {
                    defaultOutputPath = item.release.folder;
                } else if (item.relWithDebInfo.checked) {
                    defaultOutputPath = item.relWithDebInfo.folder;
                } else if (item.minSizeRel.checked) {
                    defaultOutputPath = item.minSizeRel.folder;
                }
            }
        }
    }
    return defaultOutputPath;
}

void ConfigureProjPane::slotConfigureDone()
{
    updateKitInfo();
    save();

    dpfservice::ProjectInfo info;
    QString sourceFolder = QFileInfo(d->projectPath).path();
    info.setLanguage(d->language);
    info.setSourceFolder(sourceFolder);
    info.setKitName(CmakeGenerator::toolKitName());
    //    info.setBuildFolder(outputPath);
    info.setWorkspaceFolder(sourceFolder);
    info.setProjectFilePath(d->projectPath);

    QString type, path;
    getSelectedItem(type, path);
    info.setBuildType(type);
    info.setBuildFolder(path);
    info.setBuildProgram("cmake");

    QStringList arguments;
    arguments << "-S";
    arguments << info.sourceFolder();
    arguments << "-B";
    arguments << info.buildFolder();
    arguments << "-G";
    arguments << CDT_PROJECT_KIT::get()->CDT4_GENERATOR;
    arguments << "-DCMAKE_BUILD_TYPE=" + info.buildType();
    arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
    arguments << info.buildCustomArgs();
    info.setBuildCustomArgs(arguments);

    emit configureDone(info);
}

void ConfigureProjPane::slotBrowseBtnClicked()
{
    auto sender = QObject::sender();
    QString objName = sender->objectName();

    QString outputDirectory = QFileDialog::getExistingDirectory(this, "Output directory");
    if (!outputDirectory.isEmpty()) {
        QLineEdit *lineEdit = nullptr;
        if (objName == "btnDebug") {
            lineEdit = d->lineEditDebug;
        } else if (objName == "btnRelease") {
            lineEdit = d->lineEditRelease;
        } else if (objName == "btnRWithDInfo") {
            lineEdit = d->lineEditRWithDInfo;
        } else if (objName == "btnMinimumSize") {
            lineEdit = d->lineEditMiniSize;
        }
        lineEdit->setText(outputDirectory.toUtf8());
    }
}

void ConfigureProjPane::slotParameterChanged()
{
    refreshParameters();
}

void ConfigureProjPane::setupUi(QWidget *widget)
{
    // center layout.
    auto horizontalLayout = new QHBoxLayout(widget);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setContentsMargins(11, 11, 11, 11);

    // content layout.
    auto btnSignalConnect = [this](QPushButton *btn) {
        connect(btn, &QPushButton::clicked, this, &ConfigureProjPane::slotBrowseBtnClicked);
    };

    auto gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    auto btnDebug = new QPushButton(widget);
    btnDebug->setText(tr("Browse..."));
    btnDebug->setObjectName(QStringLiteral("btnDebug"));
    btnSignalConnect(btnDebug);

    auto btnRelease = new QPushButton(widget);
    btnRelease->setText(tr("Browse..."));
    btnRelease->setObjectName(QStringLiteral("btnRelease"));
    btnSignalConnect(btnRelease);

    d->lineEditDebug = new QLineEdit(widget);

    auto btnRWithDInfo = new QPushButton(widget);
    btnRWithDInfo->setText(tr("Browse..."));
    btnRWithDInfo->setObjectName(QStringLiteral("btnRWithDInfo"));
    btnSignalConnect(btnRWithDInfo);

    d->cbDebug = new QRadioButton(widget);
    d->cbDebug->setText(tr("Debug"));

    d->cbRelease = new QRadioButton(widget);
    d->cbRelease->setText(tr("Release"));

    d->cbRWithDInfo = new QRadioButton(widget);
    d->cbRWithDInfo->setText(tr("Release with Debug Information"));

    d->cbMiniSize = new QRadioButton(widget);
    d->cbMiniSize->setText(tr("Minimum Size Release"));

    d->lineEditRelease = new QLineEdit(widget);

    d->cbDesktop = new QCheckBox(widget);
    d->cbDesktop->setText(kDefaultKitName);
    d->cbDesktop->setChecked(true);
    d->cbDesktop->setEnabled(false);
    connect(d->cbDesktop, &QCheckBox::stateChanged, [this](int state) {

    });

    d->lineEditMiniSize = new QLineEdit(widget);

    auto btnMinimumSize = new QPushButton(widget);
    btnMinimumSize->setText(tr("Browse..."));
    btnMinimumSize->setObjectName(QStringLiteral("btnMinimumSize"));
    btnSignalConnect(btnMinimumSize);

    d->lineEditRWithDInfo = new QLineEdit(widget);

    // add configure btn.

    auto btnConfigure = new QPushButton(widget);
    btnConfigure->setText(tr("Configure"));
    btnConfigure->connect(btnConfigure, &QPushButton::clicked, this, &ConfigureProjPane::slotConfigureDone);

    // add to layout.
    gridLayout->addWidget(btnDebug, 1, 2, 1, 1);
    gridLayout->addWidget(btnRelease, 2, 2, 1, 1);
    gridLayout->addWidget(d->lineEditDebug, 1, 1, 1, 1);
    gridLayout->addWidget(btnRWithDInfo, 3, 2, 1, 1);
    gridLayout->addWidget(d->cbDebug, 1, 0, 1, 1);
    gridLayout->addWidget(d->cbRelease, 2, 0, 1, 1);
    gridLayout->addWidget(d->cbRWithDInfo, 3, 0, 1, 1);
    gridLayout->addWidget(d->cbMiniSize, 4, 0, 1, 1);
    gridLayout->addWidget(d->lineEditRelease, 2, 1, 1, 1);
    gridLayout->addWidget(d->cbDesktop, 0, 0, 1, 1);
    gridLayout->addWidget(d->lineEditMiniSize, 4, 1, 1, 1);
    gridLayout->addWidget(btnMinimumSize, 4, 2, 1, 1);
    gridLayout->addWidget(d->lineEditRWithDInfo, 3, 1, 1, 1);
    gridLayout->addWidget(btnConfigure, 5, 2, 1, 1);

    horizontalLayout->addLayout(gridLayout);

    // leave space at right.
    auto horizontalSpacer = new QSpacerItem(200, 0, QSizePolicy::Preferred, QSizePolicy::Preferred);
    horizontalLayout->addItem(horizontalSpacer);

    connect(d->lineEditDebug, &QLineEdit::textChanged, this, &ConfigureProjPane::slotParameterChanged);
    connect(d->lineEditRelease, &QLineEdit::textChanged, this, &ConfigureProjPane::slotParameterChanged);
    connect(d->lineEditMiniSize, &QLineEdit::textChanged, this, &ConfigureProjPane::slotParameterChanged);
    connect(d->lineEditRelease, &QLineEdit::textChanged, this, &ConfigureProjPane::slotParameterChanged);

    d->group = new QButtonGroup();
    d->group->addButton(d->cbDebug, 0);
    d->group->addButton(d->cbRelease, 1);
    d->group->addButton(d->cbRWithDInfo, 2);
    d->group->addButton(d->cbMiniSize, 3);
    d->cbDebug->setChecked(true);

    connect(d->cbDesktop, &QCheckBox::stateChanged, this, &ConfigureProjPane::slotParameterChanged);
    connect(d->group, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ConfigureProjPane::slotParameterChanged);
}

void ConfigureProjPane::setAllChecked(bool checked)
{
    d->cbDebug->setChecked(checked);
    d->cbRelease->setChecked(checked);
    d->cbRWithDInfo->setChecked(checked);
    d->cbMiniSize->setChecked(checked);
    d->cbDesktop->setChecked(checked);
}

void ConfigureProjPane::useDefaultValue()
{
    if (d->projectPath.isEmpty())
        return;

    QDir folder = QFileInfo(d->projectPath).dir();
    QString folderName = folder.dirName();
    folder.cdUp();
    QString upDirectory = folder.path();

    auto folerPath = [&](QString buildType) -> QString {
        return (upDirectory + QDir::separator() + "build" + "-" + folderName + "-" + kDefaultKitName + "-" + buildType);
    };

    d->cfgItems.clear();

    ConfigureParam item;
    item.kitName = kDefaultKitName;

    item.debug.folder = folerPath("Debug");
    item.release.folder = folerPath("Release");
    item.relWithDebInfo.folder = folerPath("Release With DebugInfo");
    item.minSizeRel.folder = folerPath("Minimum Size Release");

    d->cfgItems.push_back(item);
}

void ConfigureProjPane::refreshUi()
{
    for (auto item : d->cfgItems) {
        if (item.kitName == kDefaultKitName) {
            d->cbDesktop->setChecked(item.checked);

            d->cbDebug->setChecked(item.debug.checked);
            d->lineEditDebug->setText(item.debug.folder);

            d->cbRelease->setChecked(item.release.checked);
            d->lineEditRelease->setText(item.release.folder);

            d->cbMiniSize->setChecked(item.minSizeRel.checked);
            d->lineEditMiniSize->setText(item.minSizeRel.folder);

            d->cbRWithDInfo->setChecked(item.relWithDebInfo.checked);
            d->lineEditRWithDInfo->setText(item.relWithDebInfo.folder);
        }
    }
}

void ConfigureProjPane::refreshParameters()
{
    for (auto &item : d->cfgItems) {
        if (item.kitName == kDefaultKitName) {   // TODO(mozart):just save one kit now.
            item.checked = d->cbDesktop->isChecked();
            item.debug.checked = d->cbDebug->isChecked();
            item.debug.folder = d->lineEditDebug->text();
            item.release.checked = d->cbRelease->isChecked();
            item.release.folder = d->lineEditRelease->text();
            item.minSizeRel.checked = d->cbMiniSize->isChecked();
            item.minSizeRel.folder = d->lineEditMiniSize->text();
            item.relWithDebInfo.checked = d->cbRWithDInfo->isChecked();
            item.relWithDebInfo.folder = d->lineEditRWithDInfo->text();
        }
    }
}

void ConfigureProjPane::getSelectedItem(QString &type, QString &path)
{
    if(d->cbDebug->isChecked()) {
        type = "Debug";
        path = d->lineEditDebug->text();
    } else if(d->cbRelease->isChecked()) {
        type = "Release";
        path = d->lineEditRelease->text();
    } else if(d->cbRWithDInfo->isChecked()) {
        type = "RelWithDebInfo";
        path = d->lineEditRWithDInfo->text();
    } else if(d->cbMiniSize->isChecked()) {
        type = "MinSizeRel";
        path = d->lineEditMiniSize->text();
    }
}

void ConfigureProjPane::updateKitInfo()
{
    Kit kit;
    QString outputPath = getDefaultOutputPath();
    kit.setDefaultOutput(outputPath);
    KitManager::instance()->setSelectedKit(kit);
}

QString ConfigureProjPane::configFilePath()
{
    QString cfgFilePath;

    QString newFolder = QFileInfo(d->projectPath).dir().path() + QDir::separator() + ".unioncode";
    QDir dir(newFolder);
    if (!dir.exists()) {
        dir.mkdir(newFolder);
    }
    cfgFilePath = newFolder + QDir::separator() + kConfigFileName;

    return cfgFilePath;
}

bool ConfigureProjPane::restore()
{
    QString cfgFilePath = configFilePath();

    QFile file(cfgFilePath);
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
    d->cfgItems.clear();
    for (auto key : rootObject.keys()) {
        if (!rootObject.value(key).isArray() || !rootObject.keys().contains("Configure"))
            continue;

        if (rootObject.keys().contains("Configure")) {
            QJsonArray valueArray = rootObject.value("Configure").toArray();

            for (QJsonValue value : valueArray) {
                ConfigureParam param;
                QJsonObject obj = value.toObject();

                param.kitName = obj.value("KitName").toString();
                param.checked = obj.value("KitChecked").toBool();

                QJsonObject debugObj = obj.value("Debug").toObject();
                param.debug.checked = debugObj.value("checked").toBool();
                param.debug.folder = debugObj.value("folder").toString();

                QJsonObject releaseObj = obj.value("Release").toObject();
                param.release.checked = releaseObj.value("checked").toBool();
                param.release.folder = releaseObj.value("folder").toString();

                QJsonObject relWithDebObj = obj.value("RelWithDebInfo").toObject();
                param.relWithDebInfo.checked = relWithDebObj.value("checked").toBool();
                param.relWithDebInfo.folder = relWithDebObj.value("folder").toString();

                QJsonObject minSizeRelObj = obj.value("MinSizeRel").toObject();
                param.minSizeRel.checked = minSizeRelObj.value("checked").toBool();
                param.minSizeRel.folder = minSizeRelObj.value("folder").toString();

                d->cfgItems.append(param);
            }
        }
    }
    return true;
}

bool ConfigureProjPane::save()
{
    QString cfgFilePath = configFilePath();

    auto setParam = [](QJsonObject *obj, const QString &key, const BuildTypeItem &param) {
        QJsonObject value;
        value.insert("checked", param.checked);
        value.insert("folder", param.folder);
        obj->insert(key, value);
    };

    refreshParameters();

    QJsonArray paramsArray;
    QVector<ConfigureParam>::const_iterator iter = d->cfgItems.begin();
    for (; iter != d->cfgItems.end(); ++iter) {
        const ConfigureParam &param = *iter;
        QJsonObject valueObj;
        valueObj.insert("KitName", param.kitName);
        valueObj.insert("KitChecked", param.checked);

        setParam(&valueObj, "Debug", param.debug);
        setParam(&valueObj, "Release", param.release);
        setParam(&valueObj, "RelWithDebInfo", param.relWithDebInfo);
        setParam(&valueObj, "MinSizeRel", param.minSizeRel);
        paramsArray.append(valueObj);
    }

    QJsonObject rootObject;
    rootObject.insert("Configure", paramsArray);
    QJsonDocument doc;
    doc.setObject(rootObject);
    QString jsonStr(doc.toJson(QJsonDocument::Indented));

    QFile file(cfgFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(jsonStr.toUtf8());
    file.close();

    return true;
}
