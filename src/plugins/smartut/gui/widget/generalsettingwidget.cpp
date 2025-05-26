// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generalsettingwidget.h"
#include "manager/smartutmanager.h"

#include <DStyle>
#include <DLabel>
#include <DDialog>

#include <QFileDialog>

DWIDGET_USE_NAMESPACE

GeneralSettingWidget::GeneralSettingWidget(QWidget *parent)
    : DFrame(parent)
{
    initUI();
    initConnection();
}

void GeneralSettingWidget::initUI()
{
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    testFrameworkCB = new DComboBox(this);
    templateCB = new DComboBox(this);
    templateAddBtn = new DPushButton(this);
    templateAddBtn->setIconSize({ 16, 16 });
    templateAddBtn->setIcon(DStyle::standardIcon(style(), DStyle::SP_IncreaseElement));
    templateDelBtn = new DPushButton(this);
    templateDelBtn->setIconSize({ 16, 16 });
    templateDelBtn->setIcon(DStyle::standardIcon(style(), DStyle::SP_DecreaseElement));
    templateDelBtn->setEnabled(false);

    nameFormatEdit = new DLineEdit(this);
    nameFormatEdit->setPlaceholderText("e.g.ut_${filename}.cpp");

    mainLayout->addWidget(new DLabel(tr("Test Framework"), this), 0, 0);
    mainLayout->addWidget(testFrameworkCB, 0, 1, 1, 3);
    mainLayout->addWidget(new DLabel(tr("Template"), this), 1, 0);
    mainLayout->addWidget(templateCB, 1, 1);
    mainLayout->addWidget(templateAddBtn, 1, 2);
    mainLayout->addWidget(templateDelBtn, 1, 3);
    mainLayout->addWidget(new DLabel(tr("Name Format"), this), 2, 0);
    mainLayout->addWidget(nameFormatEdit, 2, 1, 1, 3);
}

void GeneralSettingWidget::initConnection()
{
    connect(templateCB, &DComboBox::currentTextChanged, this, &GeneralSettingWidget::handleTemplateChanged);
    connect(templateAddBtn, &DPushButton::clicked, this, &GeneralSettingWidget::handleAddTemplate);
    connect(templateDelBtn, &DPushButton::clicked, this, &GeneralSettingWidget::handleDeleteTemplate);
    connect(nameFormatEdit, &DLineEdit::textChanged, this,
            [this] {
                if (nameFormatEdit->isAlert())
                    nameFormatEdit->setAlert(false);
            });
}

void GeneralSettingWidget::updateSettings()
{
    templateCB->clear();
    testFrameworkCB->clear();
    auto settings = SmartUTManager::instance()->utSetting();

    const auto &frameworks = settings->value(kGeneralGroup, kTestFrameworks).toStringList();
    testFrameworkCB->addItems(frameworks);
    const auto &activeFramework = settings->value(kActiveGroup, kActiveTestFramework).toString();
    if (frameworks.contains(activeFramework))
        testFrameworkCB->setCurrentIndex(frameworks.indexOf(activeFramework));

    const auto &tempList = settings->value(kGeneralGroup, kTemplates).toStringList();
    templateCB->addItems(tempList);
    const auto &activeTemp = settings->value(kActiveGroup, kActiveTemplate).toString();
    if (tempList.contains(activeTemp))
        templateCB->setCurrentIndex(tempList.indexOf(activeTemp));
    templateCB->insertItem(0, tr("None"));

    const auto &nameFormat = settings->value(kGeneralGroup, kNameFormat).toString();
    nameFormatEdit->setText(nameFormat);
}

void GeneralSettingWidget::handleTemplateChanged()
{
    const auto &temp = templateCB->currentText();
    const auto &defaultTemps = SmartUTManager::instance()->utSetting()->defaultValue(kGeneralGroup, kTemplates).toString();
    // None or default
    if (templateCB->currentIndex() == 0 || defaultTemps.contains(temp))
        templateDelBtn->setEnabled(false);
    else
        templateDelBtn->setEnabled(true);
}

void GeneralSettingWidget::handleAddTemplate()
{
    const auto &fileName = QFileDialog::getOpenFileName(this, tr("Select Template"), "", "Template(*.cpp)");
    if (fileName.isEmpty())
        return;

    templateCB->addItem(fileName);
    templateCB->setCurrentText(fileName);
}

void GeneralSettingWidget::handleDeleteTemplate()
{
    DDialog dlg(this);
    dlg.setIcon(QIcon::fromTheme("ide"));
    dlg.setWindowTitle(tr("Delete Template"));
    dlg.setMessage(tr("Are you sure to delete this template?"));
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("Ok", "button"), true, DDialog::ButtonRecommend);

    if (dlg.exec() == 1)
        templateCB->removeItem(templateCB->currentIndex());
}

bool GeneralSettingWidget::apply()
{
    auto setting = SmartUTManager::instance()->utSetting();
    const auto &format = nameFormatEdit->text();
    if (format.isEmpty() || !format.contains("${filename}")) {
        nameFormatEdit->setAlert(true);
        nameFormatEdit->showAlertMessage(tr("Please input a valid format, e.g.ut_${filename}.cpp"));
        nameFormatEdit->setFocus();
        nameFormatEdit->lineEdit()->selectAll();
        return false;
    }
    setting->setValue(kGeneralGroup, kNameFormat, format);

    QStringList templateList;
    for (int i = 1; i < templateCB->count(); ++i) {
        templateList << templateCB->itemText(i);
    }

    setting->setValue(kGeneralGroup, kTemplates, templateList);
    setting->setValue(kActiveGroup, kActiveTemplate, templateCB->currentIndex() == 0 ? "" : templateCB->currentText());
    setting->setValue(kActiveGroup, kActiveTestFramework, testFrameworkCB->currentText());
    return true;
}
