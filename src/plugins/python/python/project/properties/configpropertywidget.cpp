// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configpropertywidget.h"

#include "services/option/toolchaindata.h"
#include "services/option/optionmanager.h"
#include "common/toolchain/toolchain.h"
#include "common/widget/pagewidget.h"

#include <DComboBox>
#include <DLabel>
#include <DCheckBox>

#include <QVBoxLayout>

using namespace config;
using DTK_WIDGET_NAMESPACE::DComboBox;
class DetailPropertyWidgetPrivate
{
    friend class DetailPropertyWidget;
    DComboBox *pyVersionComboBox { nullptr };
    DComboBox *executeFileComboBox { nullptr };
    DCheckBox *runInTerminal { nullptr };
};

DetailPropertyWidget::DetailPropertyWidget(QWidget *parent)
    : QWidget(parent), d(new DetailPropertyWidgetPrivate())
{
    setupUI();
    initData();
}

DetailPropertyWidget::~DetailPropertyWidget()
{
    if (d)
        delete d;
}

void DetailPropertyWidget::setupUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    QHBoxLayout *hLayout = new QHBoxLayout();
    DLabel *label = new DLabel(tr("Python interpreter: "), this);
    label->setFixedWidth(120);
    d->pyVersionComboBox = new DComboBox(this);
    hLayout->addWidget(label);
    hLayout->addWidget(d->pyVersionComboBox);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    label = new DLabel(tr("Executable File: "), this);
    label->setFixedWidth(120);
    d->executeFileComboBox = new DComboBox(this);
    //todo: Provide default options in the properties window    no use for now
    d->executeFileComboBox->setEnabled(false);
    hLayout->addWidget(label);
    hLayout->addWidget(d->executeFileComboBox);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    label = new DLabel(tr("Run in terminal: "), this);
    label->setFixedWidth(120);
    d->runInTerminal = new DCheckBox(this);
    hLayout->addWidget(label);
    hLayout->addWidget(d->runInTerminal);
    vLayout->addLayout(hLayout);

    vLayout->addStretch(10);
}

/**
 * @brief findAll
 * @param pattern
 * @param str
 * @param Greedy: find all items matched when Greedy is true.
 * @return matched items.
 */
QList<QString> findAll(QString pattern, QString str, bool Greedy)
{
    QRegularExpression re(pattern);
    QRegularExpressionMatchIterator i = re.globalMatch(str);
    QStringList strList;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        strList << match.captured(1);
    }
    return strList;
}

ToolChainData::Params getSystemPython()
{
    ToolChainData tcData;
    QString retMsg;
    if (!tcData.readToolChainData(retMsg)) {
        qWarning() << retMsg;
        return {};
    }

    const auto &toolChains = tcData.getToolChanins();
    return toolChains.value(kPython);
}

void DetailPropertyWidget::initData()
{
    QVariant interpreterConfig = OptionManager::getInstance()->getValue(option::CATEGORY_PYTHON, "Interpreter");
    QVariantList variantList = interpreterConfig.toMap().value("customInterpreters").toList();
    QList<ToolChainData::ToolChainParam> customInterpreters;

    auto globalToolPath = OptionManager::getInstance()->getPythonToolPath();

    for (QVariant variant : variantList) {
        auto map = variant.toMap();
        ToolChainData::ToolChainParam interpreter { map.value("name").toString(), map.value("path").toString() };
        customInterpreters.append(interpreter);
    }

    const auto &systemPython = getSystemPython();
    int index = 0;
    for (const auto &python : systemPython) {
        QString text = python.name + "(" + python.path + ")";
        d->pyVersionComboBox->insertItem(index, text);
        d->pyVersionComboBox->setItemData(index, QVariant::fromValue(python), Qt::UserRole + 1);
        if (python.path == globalToolPath)
            d->pyVersionComboBox->setCurrentIndex(index);
        index++;
    }
    for (auto interpreter : customInterpreters) {
        d->pyVersionComboBox->insertItem(index, interpreter.name + "(" + interpreter.path + ")");
        d->pyVersionComboBox->setItemData(index, QVariant::fromValue(interpreter), Qt::UserRole + 1);
        if (interpreter.path == globalToolPath)
            d->pyVersionComboBox->setCurrentIndex(index);
        index++;
    }

    d->executeFileComboBox->insertItem(0, exeCurrent);
    d->executeFileComboBox->insertItem(1, exeEntry);
}

void DetailPropertyWidget::setValues(const config::ProjectConfigure *param)
{
    if (!param)
        return;

    int count = d->pyVersionComboBox->count();
    auto globalToolPath = OptionManager::getInstance()->getPythonToolPath();
    for (int i = 0; i < count; i++) {
        ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(d->pyVersionComboBox->itemData(i, Qt::UserRole + 1));
        if (param->pythonVersion.name == toolChainParam.name
            && param->pythonVersion.path == toolChainParam.path) {
            d->pyVersionComboBox->setCurrentIndex(i);
            break;
        }

        if (globalToolPath == toolChainParam.path)
            d->pyVersionComboBox->setCurrentIndex(i);
    }

    count = d->executeFileComboBox->count();
    for (int i = 0; i < count; i++) {
        if (param->executeFile == ExecuteFile::CURRENTFILE) {
            d->executeFileComboBox->setCurrentIndex(0);
        } else if (param->executeFile == ExecuteFile::ENTRYFILE) {
            d->executeFileComboBox->setCurrentIndex(1);
        }
    }

    d->runInTerminal->setChecked(param->runInTerminal);
}

void DetailPropertyWidget::getValues(config::ProjectConfigure *param)
{
    if (!param)
        return;

    param->pythonVersion.clear();
    int index = d->pyVersionComboBox->currentIndex();
    if (index > -1) {
        ToolChainData::ToolChainParam value = qvariant_cast<ToolChainData::ToolChainParam>(d->pyVersionComboBox->itemData(index, Qt::UserRole + 1));
        param->pythonVersion.name = value.name;
        param->pythonVersion.path = value.path;
    }

    index = d->executeFileComboBox->currentIndex();
    if (index == 0) {   //current file
        param->executeFile = ExecuteFile::CURRENTFILE;
    } else if (index == 1) {   //Entry file
        param->executeFile = ExecuteFile::ENTRYFILE;
    }

    param->runInTerminal = d->runInTerminal->isChecked();
}

class ConfigPropertyWidgetPrivate
{
    friend class ConfigPropertyWidget;

    DetailPropertyWidget *detail { nullptr };
    QStandardItem *item { nullptr };
    dpfservice::ProjectInfo projectInfo;
};

ConfigPropertyWidget::ConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, DWidget *parent)
    : PageWidget(parent), d(new ConfigPropertyWidgetPrivate())
{
    d->item = item;
    d->projectInfo = projectInfo;
    setupUI();
    initData(projectInfo);
}

ConfigPropertyWidget::~ConfigPropertyWidget()
{
    if (d)
        delete d;
}

void ConfigPropertyWidget::setupUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    d->detail = new DetailPropertyWidget();
    vLayout->addWidget(d->detail);
    vLayout->addStretch(10);
}

void ConfigPropertyWidget::initData(const dpfservice::ProjectInfo &projectInfo)
{
    auto param = ConfigUtil::instance()->getConfigureParamPointer();
    d->detail->setValues(param);
    param->kit = projectInfo.kitName();
    param->language = projectInfo.language();
    param->projectPath = projectInfo.workspaceFolder();
}

void ConfigPropertyWidget::saveConfig()
{
    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    d->detail->getValues(param);

    QString filePath = ConfigUtil::instance()->getConfigPath(param->projectPath);
    ConfigUtil::instance()->saveConfig(filePath, *param);

    ConfigUtil::instance()->updateProjectInfo(d->projectInfo, param);
    dpfservice::ProjectInfo::set(d->item, d->projectInfo);
}
