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
    this->setLineWidth(0);
    d->templatePath = templatePath;
    if (!TemplateParser::readWizardConfig(d->templatePath, d->wizardInfo))
        return;
    QVBoxLayout *vLayout = new QVBoxLayout();
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
    DFrame *separator = new DFrame;
    separator->setFrameShape(QFrame::VLine);
    separator->setMaximumHeight(28);
    

    bottomLayout->addStretch(15);
    bottomLayout->addWidget(cancel);
    bottomLayout->addWidget(separator);
    bottomLayout->addWidget(create);

    connect(create, &DSuggestButton::clicked, [&](){
            PojectGenParam param;
            if(this->getGenParams(param))
                generate(param);
    });

    connect(cancel, &DPushButton::clicked, this , &DetailWidget::closeSignal);

    //调整窗口的上边距
    vLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    auto iter = d->wizardInfo.configures.begin();
    for (; iter != d->wizardInfo.configures.end(); ++iter) {
        QHBoxLayout *hLayout = new QHBoxLayout();
        QFormLayout *fLayout = new QFormLayout();

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
            DLineEdit *lineEdit = new DLineEdit();
            if (!iter->defaultValues.isEmpty()) {
                lineEdit->setText(iter->defaultValues.at(0));
            }
            hLayout->addWidget(d->label, 0, Qt::AlignRight);
            hLayout->addWidget(lineEdit,Qt::AlignCenter);
            hLayout->setSpacing(10);

            d->lineEditMap.insert(iter->key, lineEdit);
            if (iter->browse) {
                lineEdit->setFixedSize(244, 36);
                lineEdit->lineEdit()->setReadOnly(true);  //设置lineedit是否为只读模式

                DSuggestButton *browse = new DSuggestButton("...");
                browse->setFixedSize(36, 36);
                hLayout->addWidget(browse, 0);
                lineEdit->lineEdit()->setAlignment(Qt::AlignCenter);
                hLayout->setContentsMargins(0, 0, 0, 0);

                connect(browse, &DPushButton::clicked, [=]() {
                    QString path = DFileDialog::getExistingDirectory(this, tr("Choose path"), QDir::homePath());
                    if (!path.isEmpty()) {
                        lineEdit->setText(path);
                    }
                });
            } else {
                lineEdit->setFixedSize(290, 36);
            }
            // 使用传值方式捕获迭代器和displayName
            auto displayName = iter->displayName;
            connect(create, &DSuggestButton::clicked, [this, lineEdit, displayName](){
                 if(!lineEdit->text().isEmpty()) {
                     return;
                 }
                 QString alertMsg;
                 if (displayName == "File Name")
                    alertMsg = tr("The filename can't be empty!");
                 else if(displayName == "Project Name")
                    alertMsg = tr("The project can't be empty!");
                 else if(displayName == "Location")
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

        fLayout->addRow(hLayout);
        vLayout->addLayout(fLayout);

        // 用循环创建的二排label和lineedit， 用spacer控制上下间距
        vLayout->addSpacerItem(new QSpacerItem(0, 13, QSizePolicy::Minimum, QSizePolicy::Expanding));
    }

    //将取消和创建按钮移至窗口底部
    vLayout->addSpacerItem(new QSpacerItem(0, 160, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vLayout->addWidget(bottomWidget);
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
