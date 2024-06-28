// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "runconfigpane.h"
#include "environmentwidget.h"

#include "services/project/projectservice.h"

#include <DFrame>
#include <DLineEdit>
#include <DSuggestButton>

#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QFormLayout>
#include <QTextBrowser>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class RunConfigPanePrivate
{
    friend class RunConfigPane;

    DLineEdit *cmdArgsLineEdit{nullptr};
    DLineEdit *workingDirLineEdit{nullptr};
    DLineEdit *executableEdit{nullptr};
    QFormLayout *formLayout{nullptr};

    EnvironmentWidget *environmentWidget{nullptr};
    config::TargetRunConfigure *targetRunParam{nullptr};
    QString currentTargetName;
};

RunConfigPane::RunConfigPane(QWidget *parent)
    : DWidget(parent)
    , d (new RunConfigPanePrivate())
{
    setupUi();
}

RunConfigPane::~RunConfigPane()
{

}

void RunConfigPane::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    DFrame *mainFrame = new DFrame(this);
    mainFrame->setObjectName("mainframe");
    mainFrame->setFrameShape(QFrame::Shape::NoFrame);
    d->formLayout = new QFormLayout(mainFrame);

    // excutable label ui.
    d->executableEdit = new DLineEdit(mainFrame);
    d->executableEdit->setPlaceholderText(tr("Here is the executable path"));
    connect(d->executableEdit, &DLineEdit::editingFinished, this, [this](){
        if (d->targetRunParam)
            d->targetRunParam->targetPath = d->executableEdit->text();
    });
    d->formLayout->addRow(tr("Executable path:"), d->executableEdit);
    d->formLayout->setSpacing(10);

    // command line ui.
    d->cmdArgsLineEdit = new DLineEdit(mainFrame);
    connect(d->cmdArgsLineEdit, &DLineEdit::textChanged, [this](){
        if (d->targetRunParam)
            d->targetRunParam->arguments = d->cmdArgsLineEdit->text().trimmed();
    });
    d->formLayout->addRow(tr("Command line arguments:"), d->cmdArgsLineEdit);

    // working directory ui.
    QHBoxLayout *browLayout = new QHBoxLayout(mainFrame);
    DSuggestButton *btnBrowser = new DSuggestButton(mainFrame);
    btnBrowser->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));
    btnBrowser->setIconSize(QSize(24, 24));
    btnBrowser->setFixedSize(36, 36);
    d->workingDirLineEdit = new DLineEdit(mainFrame);
    d->workingDirLineEdit->lineEdit()->setReadOnly(true);
    connect(d->workingDirLineEdit, &DLineEdit::textChanged, [this](){
        if (d->targetRunParam)
            d->targetRunParam->workDirectory = d->workingDirLineEdit->text().trimmed();
    });
    browLayout->addWidget(d->workingDirLineEdit);
    browLayout->addWidget(btnBrowser);
    d->formLayout->addRow(tr("Working directory:"), browLayout);
    connect(btnBrowser, &QPushButton::clicked, [this](){
        QString outputDirectory = QFileDialog::getExistingDirectory(this, tr("Working directory"), d->workingDirLineEdit->text());
        if (!outputDirectory.isEmpty()) {
            d->workingDirLineEdit->setText(outputDirectory.toUtf8());
        }
    });

    mainFrame->setLayout(d->formLayout);

    d->environmentWidget = new EnvironmentWidget(this, EnvType::RunCfg);
    vLayout->addWidget(mainFrame);
    vLayout->addWidget(d->environmentWidget);
    connect(d->environmentWidget, &EnvironmentWidget::envUpdated, this, [=](){
        d->environmentWidget->getValues(d->targetRunParam->env);
    });

    vLayout->setMargin(0);
}

void RunConfigPane::updateUi()
{
    d->environmentWidget->updateEnvList(&d->targetRunParam->env);
    d->cmdArgsLineEdit->setText(d->targetRunParam->arguments);
    d->workingDirLineEdit->setText(d->targetRunParam->workDirectory);
    d->currentTargetName = d->targetRunParam->targetName;
    d->executableEdit->setText(d->targetRunParam->targetPath);
}

void RunConfigPane::setTargetRunParam(config::TargetRunConfigure *targetRunParam)
{
    d->targetRunParam = targetRunParam;

    updateUi();
}

void RunConfigPane::insertTitle(DWidget *lWidget, DWidget *rWidget)
{
    if(lWidget && rWidget)
        d->formLayout->insertRow(0, lWidget, rWidget);
}
