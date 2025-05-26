// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "services/project/projectservice.h"
#include "kitspane.h"

#include <QFormLayout>

#include <DLineEdit>
#include <DComboBox>
#include <DLabel>

using namespace dpfservice;
DWIDGET_USE_NAMESPACE
KitsPane::KitsPane(const templateMgr::Page &pageInfo, DWidget *parent)
    : AbstractPane(parent), page(pageInfo)
{
    setupUi();
}

void KitsPane::setupUi()
{
    DLabel *title = new DLabel(this);
    title->setText(page.shortTitle);
    
    QFormLayout *mainLayout = new QFormLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    mainLayout->addRow(title);
    
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    
    QStringList kits = projectService->supportGeneratorName<ProjectGenerator>();

    DLabel *lb = new DLabel(tr("Kit: "), this);
    DComboBox *kitCb = new DComboBox(this);
    kitCb->addItems(kits);
    value.insert("kit", kitCb->currentText());
    mainLayout->addRow(lb, kitCb);
    
    lb = new DLabel(tr("Language: "), this);
    DComboBox *langCb = new DComboBox(this);
    auto generator = projectService->createGenerator<ProjectGenerator>(kitCb->currentText());
    langCb->addItems(generator->supportLanguages());
    value.insert("language", langCb->currentText());
    mainLayout->addRow(lb, langCb);

    connect(kitCb, &DComboBox::currentTextChanged, this, [=](const QString &text){
        value["kit"] = text;
        auto generator = projectService->createGenerator<ProjectGenerator>(kitCb->currentText());
        langCb->clear();
        langCb->addItems(generator->supportLanguages());
    });
    
    connect(langCb, &DComboBox::currentTextChanged, this, [=](const QString &text){
        value["language"] = text;
    });
}

QMap<QString, QVariant> KitsPane::getValue()
{
    return value;
}
