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
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
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

static const char *kCCompilers = "C compilers";
static const char *kCXXCompilers = "C++ compilers";
static const char *kCCXXDebuggers = "C/C++ debuggers";
static const char *kCCXXBuildSystems = "C/C++ build systems";

static const char *kNameItem = "name";
static const char *kPathItem = "path";

ToolChainData::ToolChainData()
{
}

bool ToolChainData::readToolChain(QString &filePath)
{
    // Read all contents from toolchain file.
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

    // Parse.
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qInfo() << "Toolchain file read failed!";
        return false;
    }

    auto parseSubObj = [this](QJsonObject &obj, const QString &subobjName) {
        if (obj.contains(subobjName)) {
            QJsonValue cCompilersArray = obj.value(subobjName);
            QJsonArray array = cCompilersArray.toArray();

            QStringList parameters;
            Params params;
            for (int i = 0; i < array.size(); ++i) {
                QJsonValue sub = array.at(i);
                QJsonObject subObj = sub.toObject();

                QJsonValue nameVal = subObj.value(kNameItem);
                QString name = nameVal.toString();

                QJsonValue pathVal = subObj.value(kPathItem);
                QString path = pathVal.toString();

                ToolChainParam param;
                param.name = name;
                param.path = path;

                params.push_back(param);
            }
            toolChains.insert(subobjName, params);
        }
    };

    QJsonArray array = doc.array();
    for (auto node : array) {
        auto obj = node.toObject();
        for (auto it = obj.begin(); it != obj.end(); it++) {
            parseSubObj(obj, it.key());
        }
    }

    return true;
}

const ToolChainData::ToolChains &ToolChainData::getToolChanins() const
{
    return toolChains;
}

///
/// \brief KitsManagerWidget::KitsManagerWidget
/// \param parent
///
class KitsManagerWidgetPrivate
{
    friend class KitsManagerWidget;
    QListView *listView = nullptr;
    QComboBox *cbCXXComplier = nullptr;
    QComboBox *cbCMake = nullptr;
    QComboBox *cbCComplier = nullptr;
    QComboBox *cbDebugger = nullptr;
    QLabel *labelGeneratorExpression = nullptr;
};

KitsManagerWidget::KitsManagerWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new KitsManagerWidgetPrivate)
{
    toolChainData.reset(new ToolChainData());

    // Read toolChain data.
    QString toolChainFilePath = CustomPaths::user(CustomPaths::Configures)
            + QDir::separator() + toolchains::K_TOOLCHAINFILE;
    if (QFile(toolChainFilePath).exists()) {
        bool ret = toolChainData->readToolChain(toolChainFilePath);
        qInfo() << (ret ? "read tool chain successful!" : "read tool chain failed!");
    }

    setupUi(this);
    updateUi();
}

KitsManagerWidget::~KitsManagerWidget()
{
    if (d) {
        delete d;
    }
}

void KitsManagerWidget::setupUi(QWidget *Widget)
{
    setWindowTitle(tr("Kits"));

    // Center layout.
    auto centerLayout = new QGridLayout(Widget);
    centerLayout->setSpacing(6);
    centerLayout->setContentsMargins(11, 11, 11, 11);

    // Tab widget.
    auto tabWidget = new QTabWidget(Widget);
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

    auto nameEidt = new QLineEdit(scrollAreaWidget);
    nameEidt->setText(tr("Desktop"));

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
    detailGridLayout->addWidget(nameEidt, 1, 2, 1, 1);
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

    tabWidget->addTab(tab, QString("kits"));
    auto tab_2 = new QWidget();
    tabWidget->addTab(tab_2, QString("Compilers"));

    centerLayout->addWidget(tabWidget, 0, 0, 1, 1);

    tabWidget->setCurrentIndex(0);
}

void KitsManagerWidget::updateUi()
{
    const ToolChainData::ToolChains &data = toolChainData->getToolChanins();

    // Update compiler combox.
    auto updateComplier = [](QComboBox *cb, ToolChainData::Params &params) {
        for (auto p : params) {
            QString text = p.name + "(" + p.path + ")";
            cb->addItem(text);
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
