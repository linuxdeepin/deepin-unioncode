// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitpage.h"
#include "configutil.h"
#include "cmake/option/kitmanager.h"

#include <QFormLayout>

using namespace config;
DWIDGET_USE_NAMESPACE

KitPage::KitPage(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent)
    : PageWidget(parent)
{
    this->projectInfo = projectInfo;
    this->item = item;
    initUI();
}

void KitPage::initUI()
{
    kitComboBox = new DComboBox(this);
    kitComboBox->setFixedWidth(220);

    QFormLayout *layout = new QFormLayout(this);
    layout->addRow(tr("Project Type:"), kitComboBox);
}

void KitPage::readConfig()
{
    kitComboBox->clear();

    kitComboBox->addItem(tr("None"));
    const auto &kitList = KitManager::instance()->kitList();
    for (const auto &kit : kitList) {
        kitComboBox->addItem(kit.kitName(), kit.id());
    }

    const auto &id = projectInfo.kitId();
    const auto &kit = KitManager::instance()->findKit(id);
    if (!kit.isValid()) {
        kitComboBox->setCurrentIndex(0);
    } else {
        int index = kitComboBox->findData(id);
        kitComboBox->setCurrentIndex(index);
    }
}

void KitPage::saveConfig()
{
    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    const QString id = kitComboBox->currentData().toString();
    if (id == param->kitId)
        return;

    param->kit = kitComboBox->currentText();
    param->kitId = id;
    QString filePath = ConfigUtil::instance()->getConfigPath(param->workspace);
    ConfigUtil::instance()->saveConfig(filePath, *param);

    ConfigUtil::instance()->updateProjectInfo(projectInfo, param);
    dpfservice::ProjectInfo::set(item, projectInfo);
}
