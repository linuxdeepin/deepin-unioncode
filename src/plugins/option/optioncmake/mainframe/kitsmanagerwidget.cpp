/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#include <QJsonObject>

class KitsManagerWidgetPrivate
{
    friend class KitsManagerWidget;
    QListView *listView = nullptr;
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
    centerLayout->setContentsMargins(11, 11, 11, 11);

    auto tab = new QWidget();
    auto horizontalLayout = new QHBoxLayout(tab);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setContentsMargins(11, 11, 11, 11);
    auto scrollArea = new QScrollArea(tab);
    scrollArea->setWidgetResizable(true);
    auto scrollAreaWidget = new QWidget();
    auto scrollAreaLayout = new QHBoxLayout(scrollAreaWidget);
    scrollAreaLayout->setSpacing(6);
    scrollAreaLayout->setContentsMargins(11, 11, 11, 11);
    auto verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);
    // List tree.
    d->listView = new QListView(scrollAreaWidget);
    d->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    verticalLayout->addWidget(d->listView);
    QStringList list;
    list << "Desktop";
    QStringListModel *listmodel = new QStringListModel(list);
    d->listView->setModel(listmodel);

    // Detail layout
    auto detailLayout = new QVBoxLayout();
    detailLayout->setSpacing(6);
    auto detailGridLayout = new QGridLayout();
    detailGridLayout->setSpacing(6);
    d->cbCXXComplier = new QComboBox(scrollAreaWidget);

    d->nameEidt = new QLineEdit(scrollAreaWidget);
    d->nameEidt->setText(tr("Desktop"));
    d->nameEidt->setEnabled(false);

    d->cbCMake = new QComboBox(scrollAreaWidget);

    auto labelCMake = new QLabel(scrollAreaWidget);
    labelCMake->setText(tr("CMake Tool:"));

    d->cbCComplier = new QComboBox(scrollAreaWidget);

    auto labelCompiler = new QLabel(scrollAreaWidget);
    labelCompiler->setText(tr("Compiler:"));

    d->cbDebugger = new QComboBox(scrollAreaWidget);

    auto btnCompilerManage = new QPushButton(scrollAreaWidget);
    btnCompilerManage->setText(tr("Manage..."));

    auto labelGenerator = new QLabel(scrollAreaWidget);
    labelGenerator->setText(tr("CMake generator:"));

    auto btnDebuggerManage = new QPushButton(scrollAreaWidget);
    btnDebuggerManage->setText(tr("Manage..."));

    auto labelDebugger = new QLabel(scrollAreaWidget);
    labelDebugger->setText(tr("Debugger:"));

    auto btnCMakeManage = new QPushButton(scrollAreaWidget);
    btnCMakeManage->setText(tr("Manage..."));

    d->labelGeneratorExpression = new QLabel(scrollAreaWidget);
    d->labelGeneratorExpression->setText(QString());

    auto Name = new QLabel(scrollAreaWidget);
    Name->setText(tr("Name:"));

    auto labelCCompiler = new QLabel(scrollAreaWidget);
    labelCCompiler->setText("C:");

    auto labelCXXCompiler = new QLabel(scrollAreaWidget);
    labelCXXCompiler->setText("C++:");

    detailGridLayout->addWidget(d->cbCXXComplier, 3, 2, 1, 1);
    detailGridLayout->addWidget(d->nameEidt, 1, 2, 1, 1);
    detailGridLayout->addWidget(d->cbCMake, 5, 2, 1, 1);
    detailGridLayout->addWidget(labelCMake, 5, 0, 1, 1);
    detailGridLayout->addWidget(d->cbCComplier, 2, 2, 1, 1);
    detailGridLayout->addWidget(labelCompiler, 2, 0, 2, 1);
    detailGridLayout->addWidget(d->cbDebugger, 4, 2, 1, 1);
    detailGridLayout->addWidget(btnCompilerManage, 2, 3, 2, 1);
    detailGridLayout->addWidget(labelGenerator, 6, 0, 1, 1);
    detailGridLayout->addWidget(btnDebuggerManage, 4, 3, 1, 1);
    detailGridLayout->addWidget(labelDebugger, 4, 0, 1, 1);
    detailGridLayout->addWidget(btnCMakeManage, 5, 3, 1, 1);
    detailGridLayout->addWidget(d->labelGeneratorExpression, 6, 2, 1, 1);
    detailGridLayout->addWidget(Name, 1, 0, 1, 1);
    detailGridLayout->addWidget(labelCCompiler, 2, 1, 1, 1);
    detailGridLayout->addWidget(labelCXXCompiler, 3, 1, 1, 1);

    detailLayout->addLayout(detailGridLayout);

    verticalLayout->addLayout(detailLayout);

    scrollAreaLayout->addLayout(verticalLayout);

    scrollArea->setWidget(scrollAreaWidget);

    horizontalLayout->addWidget(scrollArea);

    centerLayout->addWidget(tab, 0, 0, 1, 1);
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

    // Update Generator.
    d->labelGeneratorExpression->setText("Eclipse CDT4 - Unix Makefiles");
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
        cb->setCurrentIndex(-1);
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
