// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "templateparser.h"
#include "maindialog.h"
#include "projectpane.h"
#include "fieldspane.h"
#include "kitspane.h"
#include "services/language/languageservice.h"

#include "common/util/custompaths.h"

#include <DLabel>
#include <DComboBox>
#include <DPushButton>
#include <DMessageBox>
#include <DFileDialog>
#include <DLineEdit>
#include <DWidget>
#include <DSuggestButton>
#include <DFrame>

#include <QVBoxLayout>
#include <QPainter>
#include <QSpacerItem>
#include <QFormLayout>

DWIDGET_USE_NAMESPACE

using namespace dpfservice;
class DetailWidgetPrivate
{
    friend class DetailWidget;

    QString templatePath;
    QMap<QString, DLineEdit*> lineEditMap;
    QMap<QString, DComboBox*> comboBoxMap;
    WizardInfo wizardInfo;
    DLabel *label = nullptr;
    QList<AbstractPane *> paneList;
};

DetailWidget::DetailWidget(DWidget *parent)
    : DScrollArea(parent)
    , d(new DetailWidgetPrivate())
{

}

DetailWidget::DetailWidget(const QString &templatePath, DWidget *parent)
    : DScrollArea(parent)
    , d(new DetailWidgetPrivate())
{
    setLineWidth(0);
    d->templatePath = templatePath;
    if (!TemplateParser::readWizardConfig(d->templatePath, d->wizardInfo))
        return;
    
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(10, 0, 20, 0);
    vLayout->addSpacing(10);

    DWidget *widget = new DWidget(this);
    widget->setLayout(vLayout);

    DWidget *bottomWidget = new DWidget(this);
    QHBoxLayout * bottomLayout = new QHBoxLayout(); //创建一个按钮布局
    bottomWidget->setLayout(bottomLayout);
    bottomWidget->setContentsMargins(8, 0, 0, 0);

    DPushButton *cancel = new DPushButton(tr("Cancel"));
    cancel->setFixedSize(173, 36);
    DSuggestButton *create = new DSuggestButton(tr("Create"));
    create->setFixedSize(173, 36);

    // 创建两按钮间的分割线
    DVerticalLine *separator = new DVerticalLine(this);
    separator->setMaximumHeight(28);

    bottomLayout->addWidget(cancel);
    bottomLayout->addWidget(separator);
    bottomLayout->addWidget(create);

    connect(create, &DSuggestButton::clicked, [&](){
        PojectGenParam param;
        if(this->getGenParams(param))
        generate(param);
    });

    connect(cancel, &DPushButton::clicked, this , &DetailWidget::closeSignal);

    if (d->wizardInfo.wizardType == "QTCtype") {
        auto page = d->wizardInfo.pages.begin();
        for (; page != d->wizardInfo.pages.end(); ++page) {
            if (page->typeId == "Project") {
                auto pane = new ProjectPane(page->shortTitle, this);
                d->paneList.append(pane);
                vLayout->addWidget(pane);
            } else if (page->typeId == "Fields") {
                vLayout->addWidget(new DHorizontalLine(this));
                auto pane = new FieldsPane(*page, this);
                vLayout->addWidget(pane);
                d->paneList.append(pane);
            } else if (page->typeId == "Kits") {
                vLayout->addWidget(new DHorizontalLine(this));
                auto pane = new KitsPane(*page, this);
                vLayout->addWidget(pane);
                d->paneList.append(pane);
            }
        }
    } else {
        auto iter = d->wizardInfo.configures.begin();
        QFormLayout *fLayout = new QFormLayout();
        fLayout->setSpacing(10);

        for (; iter != d->wizardInfo.configures.end(); ++iter) {
            QHBoxLayout *hLayout = new QHBoxLayout();
            if (iter->displayName == "File Name")
                d->label = new DLabel(tr("File Name:"), this);
            else if(iter->displayName == "Project Name")
                d->label = new DLabel(tr("Project Name:"), this);
            else if(iter->displayName == "Location")
                d->label = new DLabel(tr("Location:"), this);

            d->label->setMinimumSize(55, 20);
            d->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            d->label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

            if ("lineEdit" == iter->type) {
                DLineEdit *lineEdit = new DLineEdit(this);
                if (!iter->defaultValues.isEmpty()) {
                    lineEdit->setText(iter->defaultValues.at(0));
                }
                hLayout->addWidget(lineEdit);
                hLayout->setSpacing(10);

                d->lineEditMap.insert(iter->key, lineEdit);
                if (iter->browse) {
                    lineEdit->lineEdit()->setReadOnly(true);

                    DSuggestButton *browse = new DSuggestButton("...", this);
                    browse->setFixedSize(36, 36);
                    hLayout->addWidget(browse, 0);
                    hLayout->setContentsMargins(0, 0, 0, 0);

                    connect(browse, &DPushButton::clicked, [=]() {
                        QString path = DFileDialog::getExistingDirectory(this, tr("Choose path"), QDir::homePath());
                        if (!path.isEmpty()) {
                            lineEdit->setText(path);
                        }
                    });
                }

                auto displayName = iter->displayName;
                connect(create, &DSuggestButton::clicked, [this, lineEdit, displayName](){
                     if(!lineEdit->text().isEmpty()) {
                         return;
                     }
                     QString alertMsg;
                     if (displayName == "File Name")
                        alertMsg = tr("The filename can't be empty!");
                     else if (displayName == "Project Name")
                        alertMsg = tr("The project can't be empty!");
                     else if (displayName == "Location")
                        alertMsg = tr("The address can't be empty!");

                     lineEdit->showAlertMessage(alertMsg);
                });
            } else if ("comboBox" == iter->type) {
                DComboBox *comboBox = new DComboBox(this);
                hLayout->addWidget(comboBox);

                if (!iter->defaultValues.isEmpty()) {
                    comboBox->addItems(iter->defaultValues);
                    comboBox->setCurrentIndex(0);
                }
                d->comboBoxMap.insert(iter->key, comboBox);
            }

            fLayout->addRow(d->label, hLayout);
        }
        vLayout->addLayout(fLayout);
    }
    //make the button at bottom
    vLayout->addWidget(bottomWidget);
    setWidget(widget);
    setWidgetResizable(true);
}

DetailWidget::~DetailWidget()
{
    if (d)
        delete d;
}

void DetailWidget::generate(const PojectGenParam &param)
{
    ProjectGenerate projectGen;
    PojectGenResult result;
    if (!projectGen.create(result, param)) {
        DMessageBox::critical(this, tr("error"), result.message);
        return;
    }

    if (param.type == Project) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            auto generator = service->create<LanguageGenerator>(result.kit);
            if (generator) {
                emit closeSignal();
                project.openProject(result.kit, result.language, result.projectPath);
            } else {
                DMessageBox::critical(this, tr("error"), tr("Can not find kit."));
            }
        }
    } else if (param.type == File) {
        emit closeSignal();
        editor.openFile(QString(), result.filePath);
    }
}

bool DetailWidget::getGenParams(PojectGenParam &param)
{
    foreach (auto pane, d->paneList) {
        auto value = pane->getValue();
        foreach (auto key , value.keys())
            param.settingParamMap.insert("%{" + key + "}", value.value(key).toString().trimmed());
    }
    
    foreach (auto key, d->lineEditMap.keys()) {
        auto lineEdit = d->lineEditMap.value(key);
        if (lineEdit->text().trimmed().isEmpty()) {
            return false;
        }
        param.settingParamMap.insert("%{" + key + "}", lineEdit->text().trimmed());
    }

    foreach (auto key, d->comboBoxMap.keys()) {
        auto comboBox = d->comboBoxMap.value(key);
        param.settingParamMap.insert("%{" + key + "}", comboBox->currentText().trimmed());
    }

    param.templatePath = CustomPaths::global(CustomPaths::Templates)
            + QDir::separator() + d->templatePath;
    param.generator = d->wizardInfo.generator;
    if (d->wizardInfo.type == "project") {
        param.type = Project;
    } else if (d->wizardInfo.type == "file") {
        param.type = File;
    }

    param.kit = d->wizardInfo.kit.isEmpty() ? param.settingParamMap.value("%{kit}") : getVariable(d->wizardInfo.kit, param);
    param.language = d->wizardInfo.language.isEmpty() ? param.settingParamMap.value("%{language}") : getVariable(d->wizardInfo.language, param);

    return true;
}

QString DetailWidget::getVariable(const QString &variable, PojectGenParam &param)
{
    if (variable.startsWith("%{") && param.settingParamMap.contains(variable))
        return param.settingParamMap.value(variable);
    
    return variable;
}
