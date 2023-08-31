// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitsmanagerwidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <QDebug>
#include <QDir>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QListWidget>
#include <QJsonObject>

class KitsManagerWidgetPrivate
{
    friend class KitsManagerWidget;
    QListWidget *listWidget = nullptr;
    QLineEdit *nameEidt = nullptr;
    QComboBox *cbCXXComplier = nullptr;
    QComboBox *cbCMake = nullptr;
    QComboBox *cbCComplier = nullptr;
    QComboBox *cbDebugger = nullptr;
    QLabel *labelGeneratorExpression = nullptr;

    QSharedPointer<ToolChainData> toolChainData;
};

KitsManagerWidget::KitsManagerWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new KitsManagerWidgetPrivate)
{
    d->toolChainData.reset(new ToolChainData());
    QString retMsg;
    bool ret = d->toolChainData->readToolChainData(retMsg);
    if (ret) {
        qInfo() << retMsg;
    }

    setupUi();
    updateUi();
}

KitsManagerWidget::~KitsManagerWidget()
{
    if (d) {
        delete d;
    }
}

void KitsManagerWidget::setupUi()
{
    auto centerLayout = new QGridLayout();
    centerLayout->setSpacing(6);
    centerLayout->setContentsMargins(0, 0, 11, 0);

    // List tree.
    d->listWidget = new QListWidget();
    d->listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d->listWidget->addItem("Desktop");

    auto detailGridLayout = new QGridLayout();
    detailGridLayout->setSpacing(6);
    d->cbCXXComplier = new QComboBox();

    d->nameEidt = new QLineEdit();
    d->nameEidt->setText(tr("Desktop"));
    d->nameEidt->setEnabled(false);

    d->cbCMake = new QComboBox();

    auto labelCMake = new QLabel();
    labelCMake->setText(tr("CMake Tool:"));

    d->cbCComplier = new QComboBox();

    d->cbDebugger = new QComboBox();

    auto labelGenerator = new QLabel();
    labelGenerator->setText(tr("CMake generator:CodeBlocks - Unix Makefiles"));

    auto labelDebugger = new QLabel();
    labelDebugger->setText(tr("Debugger:"));

    d->labelGeneratorExpression = new QLabel();
    d->labelGeneratorExpression->setText(QString());

    auto Name = new QLabel();
    Name->setText(tr("Name:"));

    auto labelCCompiler = new QLabel();
    labelCCompiler->setText("C Compiler:");

    auto labelCXXCompiler = new QLabel();
    labelCXXCompiler->setText("C++ Compiler:");

    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    centerLayout->addWidget(d->listWidget, 0, 0, 25, 1);
    centerLayout->addWidget(Name, 1, 1, 1, 1);
    centerLayout->addWidget(d->nameEidt, 1, 2, 1, 1);
    centerLayout->addWidget(separator, 3, 1, 1, 2);
    centerLayout->addWidget(labelCCompiler, 5, 1, 1, 1);
    centerLayout->addWidget(d->cbCComplier, 5, 2, 1, 1);
    centerLayout->addWidget(labelCXXCompiler, 7, 1, 1, 1);
    centerLayout->addWidget(d->cbCXXComplier, 7, 2, 1, 1);
    centerLayout->addWidget(labelDebugger, 9, 1, 1, 1);
    centerLayout->addWidget(d->cbDebugger, 9, 2, 1, 1);
    centerLayout->addWidget(labelCMake, 11, 1, 1, 1);
    centerLayout->addWidget(d->cbCMake, 11, 2, 1, 1);
    centerLayout->addWidget(labelGenerator, 13, 1, 1, 2);

    setLayout(centerLayout);
}

void KitsManagerWidget::updateUi()
{
    const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();

    // Update compiler combox.
    auto updateComplier = [](QComboBox *cb, ToolChainData::Params &params) {
        int i = 0;
        for (auto p : params) {
            QString text = p.name + "(" + p.path + ")";
            cb->insertItem(i, text);
            cb->setItemData(i, QVariant::fromValue(p), Qt::UserRole + 1);
            i++;
        }
    };

    ToolChainData::Params cParams = data.value(kCCompilers);
    updateComplier(d->cbCComplier, cParams);

    ToolChainData::Params cxxParams = data.value(kCXXCompilers);
    updateComplier(d->cbCXXComplier, cxxParams);

    // Update Debugger combox.
    ToolChainData::Params debugger = data.value(kCCXXDebuggers);
    updateComplier(d->cbDebugger, debugger);

    // Update CMake combox.
    ToolChainData::Params cmake = data.value(kCCXXBuildSystems);
    updateComplier(d->cbCMake, cmake);
}

bool KitsManagerWidget::getControlValue(QMap<QString, QVariant> &map)
{
    KitConfig config;
    config.name = d->nameEidt->text();

    auto comboBoxValue = [](QComboBox *cb) {
        int index = cb->currentIndex();
        if (index > -1) {
            return qvariant_cast<ToolChainData::ToolChainParam>(cb->itemData(index, Qt::UserRole + 1));
        } else {
            return ToolChainData::ToolChainParam();
        }
    };

    config.ccompiler = comboBoxValue(d->cbCComplier);
    config.cppcompiler = comboBoxValue(d->cbCXXComplier);
    config.debugger = comboBoxValue(d->cbDebugger);
    config.cmake = comboBoxValue(d->cbCMake);
    config.cmakeGenerator = d->labelGeneratorExpression->text();

    dataToMap(config, map);

    return true;
}

void KitsManagerWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    KitConfig config;
    mapToData(map, config);

    auto updateComplier = [](QComboBox *cb, ToolChainData::ToolChainParam &params) {
        for (int i = 0; i < cb->count(); i++) {
            ToolChainData::ToolChainParam data = qvariant_cast<ToolChainData::ToolChainParam>(cb->itemData(i, Qt::UserRole + 1));
            if (data.name == params.name && data.path == params.path) {
                cb->setCurrentIndex(i);
                return;
            }
        }
    };

    updateComplier(d->cbCComplier, config.ccompiler);
    updateComplier(d->cbCXXComplier, config.cppcompiler);
    updateComplier(d->cbDebugger, config.debugger);
    updateComplier(d->cbCMake, config.cmake);

    //d->labelGeneratorExpression->setText(config.cmakeGenerator);
}

bool KitsManagerWidget::dataToMap(const KitConfig &config, QMap<QString, QVariant> &map)
{
    auto updateComplier = [](QMap<QString, QVariant> &map, const QString key, const ToolChainData::ToolChainParam &params) {
        QMap<QString, QVariant> version;
        version.insert("name", params.name);
        version.insert("path", params.path);
        map.insert(key, version);
    };

    updateComplier(map, "ccompiler", config.ccompiler);
    updateComplier(map, "cppcompiler", config.cppcompiler);
    updateComplier(map, "debugger", config.debugger);
    updateComplier(map, "cmake", config.cmake);

    map.insert("name", config.name);
    map.insert("cmakeGenerator", config.cmakeGenerator);

    return true;
}

bool KitsManagerWidget::mapToData(const QMap<QString, QVariant> &map, KitConfig &config)
{
    auto updateComplier = [](const QMap<QString, QVariant> &map, const QString key, ToolChainData::ToolChainParam &params) {
        QMap<QString, QVariant> version = map.value(key).toMap();
        params.name = version.value("name").toString();
        params.path = version.value("path").toString();
    };

    updateComplier(map, "ccompiler", config.ccompiler);
    updateComplier(map, "cppcompiler", config.cppcompiler);
    updateComplier(map, "debugger", config.debugger);
    updateComplier(map, "cmake", config.cmake);

    config.name = map.value("name").toString();
    config.cmakeGenerator = map.value("cmakeGenerator").toString();

    return true;
}

void KitsManagerWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void KitsManagerWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}
