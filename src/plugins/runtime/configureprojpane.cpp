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
#include "kitmanager.h"

class ConfigureProjPanePrivate
{
    friend class ConfigureProjPane;
    QCheckBox *cbRWithDInfo = nullptr;
    QPushButton *btnDebug = nullptr;
    QLineEdit *lineEditRWithDInfo = nullptr;
    QCheckBox *cbDebug = nullptr;
    QPushButton *btnRelease = nullptr;
    QLineEdit *lineEditRelease = nullptr;
    QCheckBox *cbMiniSize = nullptr;
    QCheckBox *cbRelease = nullptr;
    QLineEdit *lineEditDebug = nullptr;
    QLineEdit *lineEditMiniSize = nullptr;
    QPushButton *btnMinimumSize = nullptr;
    QCheckBox *cbDesktop = nullptr;
    QPushButton *btnRWithDInfo = nullptr;
    QPushButton *btnConfigure = nullptr;

    QString projectPath;
};

ConfigureProjPane::ConfigureProjPane(QWidget *parent)
    : QWidget(parent)
    , d(new ConfigureProjPanePrivate)
{
    setupUi(this);
}

ConfigureProjPane::~ConfigureProjPane()
{
    if (d)
        delete d;
}

void ConfigureProjPane::setProjectPath(QString &projectPath)
{
    d->projectPath = projectPath;
    updateOutputPath();
}

ConfigureProjPane::BuildType ConfigureProjPane::getDefaultBuildType() const
{
    // TODO(Mozart)
    return {};
}

QString ConfigureProjPane::getDefaultOutputPath() const
{
    QString ret;
    if (d->cbDebug->isChecked()) {
        ret = d->lineEditDebug->text();
    } else if (d->cbRelease->isChecked()) {
        ret = d->lineEditRelease->text();
    } else if (d->cbRWithDInfo->isChecked()) {
        ret = d->lineEditRWithDInfo->text();
    } else if (d->cbMiniSize->isChecked()) {
        ret = d->lineEditMiniSize->text();
    } else {
        ret = "../build-Default";
    }
    return ret;
}

void ConfigureProjPane::slotConfigureDone()
{
    updateKitInfo();
    emit configureDone();
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

void ConfigureProjPane::setupUi(QWidget *widget)
{
    // center layout.
    auto horizontalLayout = new QHBoxLayout(widget);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setContentsMargins(11, 11, 11, 11);

    // content layout.
    auto btnSignalConnect = [this](QPushButton *btn){
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

    d->cbDebug = new QCheckBox(widget);
    d->cbDebug->setText(tr("Debug"));

    d->cbRelease = new QCheckBox(widget);
    d->cbRelease->setText(tr("Release"));

    d->cbRWithDInfo = new QCheckBox(widget);
    d->cbRWithDInfo->setText(tr("Release with Debug Information"));

    d->cbMiniSize = new QCheckBox(widget);
    d->cbMiniSize->setText(tr("Minimum Size Release"));

    d->lineEditRelease = new QLineEdit(widget);

    d->cbDesktop = new QCheckBox(widget);
    d->cbDesktop->setText(tr("Desktop"));
    connect(d->cbDesktop, &QCheckBox::stateChanged, [this](int state){
        this->setAllChecked(state);
    });

    setAllChecked(true);

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
}

void ConfigureProjPane::setAllChecked(bool checked)
{
    d->cbDebug->setChecked(checked);
    d->cbRelease->setChecked(checked);
    d->cbRWithDInfo->setChecked(checked);
    d->cbMiniSize->setChecked(checked);
    d->cbDesktop->setChecked(checked);
}

void ConfigureProjPane::updateOutputPath()
{
    if (d->projectPath.isEmpty())
        return;

    QDir folder = QFileInfo(d->projectPath).dir();
    QString folderName = folder.dirName();
    folder.cdUp();
    QString upDirectory = folder.path();
    QString kit = "Desktop";

    auto fillLineEdit = [&](QString &buildType, QLineEdit *lineEdit) {
        QString text = upDirectory + QDir::separator() + "build" + "-" + folderName + "-" + kit + "-" + buildType;
        if (lineEdit) {
            lineEdit->setText(text);
        }
    };

    QString buildType = "Debug";
    fillLineEdit(buildType, d->lineEditDebug);
    buildType = "Release";
    fillLineEdit(buildType, d->lineEditRelease);
    buildType = "Release With DebugInfo";
    fillLineEdit(buildType, d->lineEditRWithDInfo);
    buildType = "Minimum Size Release";
    fillLineEdit(buildType, d->lineEditMiniSize);
}

void ConfigureProjPane::updateKitInfo()
{
    Kit kit;
    QString outputPath = getDefaultOutputPath();
    kit.setDefaultOutput(outputPath);
    KitManager::instance()->setSelectedKit(kit);
}
