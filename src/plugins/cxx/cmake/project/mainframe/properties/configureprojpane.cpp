// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configureprojpane.h"

#include "cmake/project/mainframe/cmakeasynparse.h"
#include "cmake/project/mainframe/cmakeprojectgenerator.h"

#include "services/option/optionmanager.h"

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
#include <QComboBox>

// TODO(mozart):should get from kitmanager.
static const char *kDefaultKitName = "Desktop";

using namespace config;

class ConfigureProjPanePrivate
{
    friend class ConfigureProjPane;

    QComboBox *kitComboBox{nullptr};

    QRadioButton *radioDebug{nullptr};
    QRadioButton *radioRelease{nullptr};

    QLineEdit *lineEditDebug{nullptr};
    QLineEdit *lineEditRelease{nullptr};

    QPushButton *btnRWithDInfo{nullptr};
    QPushButton *btnConfigure{nullptr};

    QButtonGroup *group{nullptr};

    ConfigureParam *cfgItem{nullptr};
};

ConfigureProjPane::ConfigureProjPane(const QString &language,
                                     const QString &workspace,
                                     QWidget *parent)
    : QWidget(parent)
    , d(new ConfigureProjPanePrivate)
{
    d->cfgItem = ConfigUtil::instance()->getConfigureParamPointer();
    d->cfgItem->clear();
    d->cfgItem->language = language;
    d->cfgItem->workspace = workspace;

    setupUI();
    updateUI();

    connect(ConfigUtil::instance(), QOverload<const dpfservice::ProjectInfo &>::of(&ConfigUtil::configureDone),
            [this](const dpfservice::ProjectInfo &info) {
        QString propertyPath = ConfigUtil::instance()->getConfigPath(d->cfgItem->workspace);
        config::ConfigUtil::instance()->saveConfig(propertyPath, *d->cfgItem);
        dpfservice::ProjectInfo projectInfo = info;
        config::ConfigUtil::instance()->updateProjectInfo(projectInfo, d->cfgItem);
        emit configureDone(projectInfo);
    });
}

ConfigureProjPane::~ConfigureProjPane()
{
    if (d)
        delete d;
}

void ConfigureProjPane::setupUI()
{
    auto btnSignalConnect = [this](QPushButton *btn, QLineEdit *lineEdit){
        connect(btn, &QPushButton::clicked, [=](){
            QString outputDirectory = QFileDialog::getExistingDirectory(this, "Output directory");
            if (!outputDirectory.isEmpty()) {
                lineEdit->setText(outputDirectory);
            }
        });
    };

    QLabel *kitLabel = new QLabel(tr("Select kit: "));
    kitLabel->setFixedWidth(100);
    d->kitComboBox = new QComboBox();
    d->kitComboBox->addItem(kDefaultKitName);
    d->kitComboBox->setCurrentIndex(0);
    QHBoxLayout *hLayoutKit = new QHBoxLayout();
    hLayoutKit->addWidget(kitLabel, 0, Qt::AlignLeft);
    hLayoutKit->addWidget(d->kitComboBox, 0, Qt::AlignLeft);
    hLayoutKit->addStretch(10);

    d->radioDebug = new QRadioButton("Debug");
    d->radioDebug->setFixedWidth(100);
    auto btnDebug = new QPushButton(tr("Browse..."));
    d->lineEditDebug = new QLineEdit();
    d->lineEditDebug->setMinimumWidth(400);
    btnSignalConnect(btnDebug, d->lineEditDebug);
    QHBoxLayout *hLayoutDebug = new QHBoxLayout();
    hLayoutDebug->addWidget(d->radioDebug);
    hLayoutDebug->addWidget(d->lineEditDebug);
    hLayoutDebug->addWidget(btnDebug);

    d->radioRelease = new QRadioButton("Release");
    d->radioRelease->setFixedWidth(100);
    auto btnRelease = new QPushButton(tr("Browse..."));
    d->lineEditRelease = new QLineEdit();
    d->lineEditDebug->setMinimumWidth(400);
    btnSignalConnect(btnRelease, d->lineEditRelease);
    QHBoxLayout *hLayoutRelease = new QHBoxLayout();
    hLayoutRelease->addWidget(d->radioRelease);
    hLayoutRelease->addWidget(d->lineEditRelease);
    hLayoutRelease->addWidget(btnRelease);

    auto btnConfigure = new QPushButton(tr("Configure"));
    btnConfigure->connect(btnConfigure, &QPushButton::clicked,
                          this, &ConfigureProjPane::slotConfigure);

    QHBoxLayout *hLayoutBottom = new QHBoxLayout();
    hLayoutBottom->addStretch(10);
    hLayoutBottom->addWidget(btnConfigure, 0, Qt::AlignRight);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayoutKit);
    vLayout->addSpacing(10);
    vLayout->addLayout(hLayoutDebug);
    vLayout->addLayout(hLayoutRelease);
    vLayout->addSpacing(10);
    vLayout->addLayout(hLayoutBottom);
    setLayout(vLayout);

    d->group = new QButtonGroup();
    d->group->addButton(d->radioDebug, 0);
    d->group->addButton(d->radioRelease, 1);
    d->radioDebug->setChecked(true);
}

void ConfigureProjPane::resetUI()
{
    d->kitComboBox->clear();
    d->radioDebug->setChecked(true);
    d->lineEditDebug->clear();
    d->lineEditRelease->clear();
}

void ConfigureProjPane::updateUI()
{
    resetUI();

    d->kitComboBox->addItem(kDefaultKitName);

    if (d->cfgItem->workspace.isEmpty())
        return;

    QDir folder(d->cfgItem->workspace);
    QString folderName = folder.dirName();
    folder.cdUp();
    QString upDirectory = folder.path();

    auto folerPath = [=](QString buildType) -> QString {
        return (upDirectory + QDir::separator() + "build" + "-" + folderName + "-" + kDefaultKitName + "-" + buildType);
    };

    d->lineEditDebug->setText(folerPath(ConfigUtil::instance()->getNameFromType(Debug)));
    d->lineEditRelease->setText(folerPath(ConfigUtil::instance()->getNameFromType(Release)));
}

void ConfigureProjPane::slotConfigure()
{
    QList<ConfigType> initType = {Debug, Release};
    foreach (auto type, initType) {
        BuildConfigure buildConfigure;
        buildConfigure.type = type;
        // init output directory
        if (Debug == type) {
            buildConfigure.directory = d->lineEditDebug->text().trimmed();
        } else if (Release == type) {
            buildConfigure.directory = d->lineEditRelease->text().trimmed();
        }
        // init build steps
        for (int i = 0; i < StepCount; i++) {
            StepItem item;
            item.type = static_cast<StepType>(i);
            buildConfigure.steps.push_back(item);
        }
        // init environment
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        foreach (auto key, env.keys()) {
            buildConfigure.env.environments.insert(key, env.value(key));
        }

        d->cfgItem->buildConfigures.push_back(buildConfigure);
    }

    d->cfgItem->defaultType = ConfigUtil::instance()->getTypeFromName(d->group->checkedButton()->text());
    d->cfgItem->tempSelType = d->cfgItem->defaultType;
    d->cfgItem->kit = d->kitComboBox->currentText();

    ConfigUtil::instance()->configProject(d->cfgItem);
}
