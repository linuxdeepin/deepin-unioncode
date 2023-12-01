// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "templateparser.h"
#include "maindialog.h"
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

#include <QVBoxLayout>
#include <QPainter>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE

using namespace dpfservice;
class DetailWidgetPrivate
{
    friend class DetailWidget;

    QString templatePath;
    QMap<QString, DLineEdit*> lineEditMap;
    QMap<QString, DComboBox*> comboBoxMap;
    WizardInfo wizardInfo;
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
    d->templatePath = templatePath;
    if (!TemplateParser::readWizardConfig(d->templatePath, d->wizardInfo))
        return;

    DWidget *widget = new DWidget();

    QVBoxLayout *vLayout = new QVBoxLayout();
    widget->setLayout(vLayout);

    DLabel *titleLabel = new DLabel(d->wizardInfo.trDisplayName);
    vLayout->addSpacing(10);

    DWidget *bottomWidget = new DWidget();
    QHBoxLayout * bottomLayout = new QHBoxLayout(); //创建一个按钮布局
    bottomWidget->setLayout(bottomLayout);

    DPushButton *cancel = new DPushButton(tr("Cancel"));
    cancel->setFixedSize(200,46);
    DSuggestButton *create = new DSuggestButton(tr("Create"));
    create->setFixedSize(200,46);

    bottomLayout->addStretch(15);
    bottomLayout->addWidget(cancel,Qt::AlignLeft);
    bottomLayout->addWidget(create);

    connect(create, &DSuggestButton::clicked, [&](){
            PojectGenParam param;
            if(this->getGenParams(param))
                generate(param);

    });

    connect(cancel, &DPushButton::clicked, this , &DetailWidget::closeSignal);

    auto iter = d->wizardInfo.configures.begin();
    for (; iter != d->wizardInfo.configures.end(); ++iter) {
        QHBoxLayout *hLayout = new QHBoxLayout();
        QString  str = iter->displayName;

        DLabel *label = new DLabel(tr(str.toStdString().c_str()) + ":");

        label->setFixedSize(100, 20);
        label->setContentsMargins(8,0,0,0);
        label->setAlignment(Qt::AlignLeft);

        hLayout->addWidget(label);


        if ("lineEdit" == iter->type) {
            DLineEdit *lineEdit = new DLineEdit();
            if (!iter->defaultValues.isEmpty()) {
                lineEdit->setText(iter->defaultValues.at(0));
            }

            hLayout->addWidget(lineEdit,Qt::AlignRight);

            d->lineEditMap.insert(iter->key, lineEdit);
            if (iter->browse) {
                lineEdit->setFixedSize(370, 36);
                lineEdit->setContentsMargins(3,0,0,0);
                lineEdit->lineEdit()->setReadOnly(true);  //设置lineedit是否为只读模式

                DSuggestButton *browse = new DSuggestButton("...");
                browse->setFixedSize(36, 36);
                hLayout->addWidget(browse, 0, Qt::AlignRight);
                hLayout->setStretchFactor(browse, 1);

                connect(browse, &DPushButton::clicked, [=]() {
                    QString path = DFileDialog::getExistingDirectory(this, tr("Choose path"), QDir::homePath());
                    if (!path.isEmpty()) {
                        lineEdit->setText(path);
                    }
                });
            } else {
                lineEdit->setFixedSize(405, 36);

            }

            connect(create, &DSuggestButton::clicked, [this, lineEdit,str](){
                 if(!lineEdit->text().isEmpty()) {
                     return;
                 }
                 QString alertMsg;
                 if (str == "File Name")
                    alertMsg = tr("The filename can't be empty!");
                 else if(str == "Project Name")
                    alertMsg = tr("The project can't be empty!");
                 else if(str == "Location")
                    alertMsg = tr("The address can't be empty!");

                 lineEdit->showAlertMessage(alertMsg);
            });

        } else if ("comboBox" == iter->type) {
            DComboBox *comboBox = new DComboBox();
            comboBox->setFixedSize(400, 30);
            hLayout->addWidget(comboBox, 0, Qt::AlignLeft);
            hLayout->setStretchFactor(comboBox, 3);

            if (!iter->defaultValues.isEmpty()) {
                comboBox->addItems(iter->defaultValues);
                comboBox->setCurrentIndex(0);
            }
            d->comboBoxMap.insert(iter->key, comboBox);
        }

        vLayout->addLayout(hLayout);

        for (int i = 0; i < 2; ++i) {
            QWidget *blankWidget = new QWidget;
            vLayout->addWidget(blankWidget);
        }
        vLayout->setContentsMargins(0,30,0,0);

    }
    //将取消和创建按钮移至窗口底部
    vLayout->addSpacerItem(new QSpacerItem(20, 230, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vLayout->addWidget(bottomWidget,Qt::AlignLeft);
    setWidget(widget);
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
        editor.openFile(result.filePath);
    }
}

bool DetailWidget::getGenParams(PojectGenParam &param)
{
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
    param.kit = d->wizardInfo.kit;
    param.language = d->wizardInfo.language;

    return true;
}



