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

#include <QVBoxLayout>

using namespace config;
class DetailPropertyWidgetPrivate
{
    friend class DetailPropertyWidget;
    DComboBox *pyVersionComboBox{nullptr};
    QSharedPointer<ToolChainData> toolChainData;
};

DetailPropertyWidget::DetailPropertyWidget(QWidget *parent)
    : QWidget(parent)
    , d(new DetailPropertyWidgetPrivate())
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
    DLabel *label = new DLabel(DLabel::tr("Python interpreter: "));
    label->setFixedWidth(120);
    d->pyVersionComboBox = new DComboBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->pyVersionComboBox);
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
    QRegExp rxlen(pattern);
    rxlen.setMinimal(Greedy);
    int position = 0;
    QList<QString> strList;
    while (position >= 0) {
        position = rxlen.indexIn(str, position);
        if (position < 0)
            break;
        strList << rxlen.cap(1);
        position += rxlen.matchedLength();
    }
    return strList;
}

QStringList getPythonAllVersion()
{
    QDir dir("/usr/bin");
    QStringList filter { "Python*.*" };
    dir.setNameFilters(filter);
    QStringList pythonList = dir.entryList();

    QString pattern = "((\\d)|(\\d+.\\d+))($|\\s)";
    QStringList versions = findAll(pattern, pythonList.join(" "), true);
    return versions;
}

QString getNewVersionPython()
{
    QStringList versions = getPythonAllVersion();
    double newVersion = 0;
    for (auto version : versions) {
        double v = version.toDouble();
        if (v > newVersion) {
            newVersion = v;
        }
    }
    QString pythonCmd = "python" + QString::number(newVersion);
    return pythonCmd;
}

void DetailPropertyWidget::initData()
{
    d->toolChainData.reset(new ToolChainData());
    auto initComboBox = [](DComboBox *comboBox, const QStringList &data) {
        int index = 0;
        for (auto version : data) {
            ToolChainData::ToolChainParam param;
            param.name =  QString("python%1").arg(version);
            param.path = "/usr/bin/" + param.name;
            QString text = param.name + "(" + param.path + ")";
            comboBox->insertItem(index, text);
            comboBox->setItemData(index, QVariant::fromValue(param), Qt::UserRole + 1);
            index++;
        }
        comboBox->setCurrentText(getNewVersionPython());
    };
    initComboBox(d->pyVersionComboBox, getPythonAllVersion());
}

void DetailPropertyWidget::setValues(const config::ProjectConfigure *param)
{
    if (!param)
        return;

    auto initComboBox = [](DComboBox *comboBox, const config::ItemInfo &itemInfo) {
        int count = comboBox->count();
        auto globalToolPath = OptionManager::getInstance()->getPythonToolPath();
        for (int i = 0; i < count; i++) {
            ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(comboBox->itemData(i, Qt::UserRole + 1));
            if ((itemInfo.name == toolChainParam.name
                       && itemInfo.path == toolChainParam.path)
                       || globalToolPath == toolChainParam.path) {
                comboBox->setCurrentIndex(i);
                break;
            }
        }
    };
    initComboBox(d->pyVersionComboBox, param->pythonVersion);
}

void DetailPropertyWidget::getValues(config::ProjectConfigure *param)
{
    if (!param)
        return;

    auto getValue = [](DComboBox *comboBox, ItemInfo &itemInfo){
        itemInfo.clear();
        int index = comboBox->currentIndex();
        if (index > -1) {
            ToolChainData::ToolChainParam value = qvariant_cast<ToolChainData::ToolChainParam>(comboBox->itemData(index, Qt::UserRole + 1));
            itemInfo.name = value.name;
            itemInfo.path = value.path;
        }
    };

    getValue(d->pyVersionComboBox, param->pythonVersion);
}

class ConfigPropertyWidgetPrivate
{
    friend class ConfigPropertyWidget;

    DetailPropertyWidget *detail{nullptr};
    QStandardItem *item{nullptr};
    dpfservice::ProjectInfo projectInfo;
};

ConfigPropertyWidget::ConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, DWidget *parent)
    : PageWidget(parent)
    , d(new ConfigPropertyWidgetPrivate())
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
