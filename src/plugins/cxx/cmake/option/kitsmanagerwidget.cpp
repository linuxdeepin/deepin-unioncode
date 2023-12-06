// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitsmanagerwidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <DLabel>
#include <DFrame>
#include <DLineEdit>
#include <DListView>
#include <DPushButton>
#include <DComboBox>
#include <DWidget>
#include <DTabWidget>
#include <DIconButton>

#include <QDebug>
#include <QDir>
#include <QtCore/QVariant>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QJsonObject>

DWIDGET_USE_NAMESPACE

class KitsManagerWidgetPrivate
{
    friend class KitsManagerWidget;
    DListView *listView = nullptr;
    DLineEdit *nameEidt = nullptr;
    DComboBox *cbCXXComplier = nullptr;
    DComboBox *cbCMake = nullptr;
    DComboBox *cbCComplier = nullptr;
    DComboBox *cbDebugger = nullptr;
    DComboBox *cbGenerator = nullptr;
    DLabel *labelGeneratorExpression = nullptr;
    QStringListModel *listModel = nullptr;

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
    auto centerLayout = new QHBoxLayout(this);
    centerLayout->setSpacing(6);

    // leftSide
    auto listFrame = new DFrame(this);
    auto listlayout = new QVBoxLayout(listFrame);

    d->listView = new DListView(this);
    d->listView->setAlternatingRowColors(true);
    d->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    DFrame* separator = new DFrame(this);
    separator->setFrameShape(DFrame::HLine);
    separator->setFrameShadow(DFrame::Plain);

    DIconButton *iconBtnI = new DIconButton(DStyle::SP_IncreaseElement, this);
    iconBtnI->setFixedSize(QSize(20,20));
    iconBtnI->setFlat(true);
    DIconButton *iconBtnD = new DIconButton(DStyle::SP_DecreaseElement, this);
    iconBtnD->setFixedSize(QSize(20,20));
    iconBtnD->setFlat(true);

    listlayout->addWidget(d->listView);
    listlayout->addWidget(separator);

    auto btnlayout = new QHBoxLayout;
    btnlayout->addWidget(iconBtnI);
    btnlayout->addWidget(iconBtnD);
    btnlayout->setAlignment(Qt::AlignLeft);

    listlayout->addLayout(btnlayout);

    d->listModel = new QStringListModel(this);
    d->listView->setModel(d->listModel);
    d->listModel->setStringList(QStringList() << tr("Desktop"));

    //rightSide
    auto detailGridLayout = new QGridLayout;
    detailGridLayout->setSpacing(6);
    d->cbCXXComplier = new DComboBox();

    d->nameEidt = new DLineEdit(this);
    d->nameEidt->setText(tr("Desktop"));
    d->nameEidt->setEnabled(false);

    d->cbCMake = new DComboBox(this);

    auto labelCMake = new DLabel(this);
    labelCMake->setText(tr("CMake Tool:"));

    d->cbCComplier = new DComboBox(this);

    d->cbDebugger = new DComboBox(this);

    auto labelGenerator = new DLabel(this);
    labelGenerator->setText(tr("CMake generator:"));
    d->cbGenerator = new DComboBox(this);
    d->cbGenerator->insertItem(0, "CodeBlocks - Unix Makefiles");
    d->cbGenerator->setEnabled(false);

    auto labelDebugger = new DLabel(this);
    labelDebugger->setText(tr("Debugger:"));

    d->labelGeneratorExpression = new DLabel(this);
    d->labelGeneratorExpression->setText(QString());

    auto Name = new DLabel(this);
    Name->setText(tr("Name:"));

    auto labelCCompiler = new DLabel(this);
    labelCCompiler->setText(tr("C Compiler:"));

    auto labelCXXCompiler = new DLabel(this);
    labelCXXCompiler->setText(tr("C++ Compiler:"));

    auto formlayout = new QFormLayout;
    formlayout->addRow(Name, d->nameEidt);
    formlayout->addRow(labelCCompiler, d->cbCComplier);
    formlayout->addRow(labelCXXCompiler, d->cbCXXComplier);
    formlayout->addRow(labelDebugger, d->cbDebugger);
    formlayout->addRow(labelCMake, d->cbCMake);
    formlayout->addRow(labelGenerator, d->cbGenerator);
    //formlayout->setContentsMargins(0, 0, 10, 0);

    centerLayout->addWidget(listFrame);
    centerLayout->addSpacing(10);
    centerLayout->addLayout(formlayout);

    setLayout(centerLayout);
}

void KitsManagerWidget::updateUi()
{
    const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();

    // Update compiler combox.
    auto updateComplier = [](DComboBox *cb, ToolChainData::Params &params) {
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

    auto comboBoxValue = [](DComboBox *cb) {
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

    auto updateComplier = [](DComboBox *cb, ToolChainData::ToolChainParam &params) {
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
