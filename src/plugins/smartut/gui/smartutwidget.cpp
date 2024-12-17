// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smartutwidget.h"
#include "settingdialog.h"
#include "projecttreeview.h"
#include "manager/smartutmanager.h"
#include "utils/utils.h"
#include "event/eventreceiver.h"

#include <DLabel>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QFileInfo>
#include <QDir>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

SmartUTWidget::SmartUTWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnection();
}

void SmartUTWidget::showSettingDialog()
{
    if (!settingDlg)
        settingDlg = new SettingDialog(this);

    if (settingDlg->exec() == QDialog::Accepted) {
        const auto &fileList = settingDlg->selectedFileList();
        fillProjectView(settingDlg->selectedProject(), fileList);
    }
}

void SmartUTWidget::initUI()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    mainWidget = new QStackedWidget(this);
    mainWidget->addWidget(createBlankPage());
    mainWidget->addWidget(createMainWidget());

    layout->addWidget(mainWidget);
}

void SmartUTWidget::initConnection()
{
    connect(generateBtn, &DToolButton::clicked, this, qOverload<>(&SmartUTWidget::createUTFiles));
    connect(stopBtn, &DToolButton::clicked, SmartUTManager::instance(), qOverload<>(&SmartUTManager::stop));
    connect(prjView, &ProjectTreeView::reqGenerateUTFile, this, qOverload<NodeItem *>(&SmartUTWidget::createUTFiles));
    connect(prjView, &ProjectTreeView::reqStopGenerate, SmartUTManager::instance(), qOverload<NodeItem *>(&SmartUTManager::stop));
    connect(SmartUTManager::instance(), &SmartUTManager::itemStateChanged, this, &SmartUTWidget::updateItemState);
    connect(EventDistributeProxy::instance(), &EventDistributeProxy::sigLLMCountChanged, this, &SmartUTWidget::updateModelList);
}

QWidget *SmartUTWidget::createBlankPage()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    DLabel *configureLabel = new DLabel(this);
    configureLabel->setAlignment(Qt::AlignCenter);

    DLabel *titleLabel = new DLabel(tr("The current resource is not configured"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);

    DLabel *msgLabel = new DLabel(this);
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->setWordWrap(true);

    auto updateIcon = [configureLabel, msgLabel]() {
        configureLabel->setPixmap(QIcon::fromTheme("uc_configure").pixmap({ 234, 144 }));
        QString msgFormat = tr("<html><head/><body><p>Please click the Setting button \"%1\""
                               " in the upper right corner to configure</p></body></html>");
        QString icon = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
                ? "<img src=\":/icons/deepin/builtin/texts/uc_settings_16px.svg\"/>"
                : "<img src=\":/icons/deepin/builtin/dark/icons/uc_settings-dark.svg\"/>";
        msgLabel->setText(msgFormat.arg(icon));
    };
    updateIcon();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=] { updateIcon(); });

    layout->addStretch(1);
    layout->addWidget(configureLabel);
    layout->addSpacing(50);
    layout->addWidget(titleLabel);
    layout->addWidget(msgLabel);
    layout->addStretch(2);

    return widget;
}

QWidget *SmartUTWidget::createMainWidget()
{
    auto createButton = [this](const QString &icon, const QString &tips) {
        auto btn = new DToolButton(this);
        btn->setIconSize({ 16, 16 });
        btn->setIcon(QIcon::fromTheme(icon));
        btn->setToolTip(tips);
        return btn;
    };

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    prjView = new ProjectTreeView(ProjectTreeView::UnitTest, this);
    modelCB = new DComboBox(this);
    modelCB->addItems(SmartUTManager::instance()->modelList());
    generateBtn = createButton("uc_generate", tr("Generate unit test files"));
    runBtn = createButton("uc_run", tr("Run"));
    reportBtn = createButton("uc_report", tr("Generate coverage report"));
    stopBtn = createButton("uc_stop", tr("Stop"));
    stopBtn->setVisible(false);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setContentsMargins(10, 10, 10, 10);
    bottomLayout->addWidget(new DLabel(tr("Select Model:"), this));
    bottomLayout->addWidget(modelCB, 1);
    bottomLayout->addWidget(generateBtn);
    bottomLayout->addWidget(stopBtn);
    bottomLayout->addWidget(runBtn);
    bottomLayout->addWidget(reportBtn);

    layout->addWidget(prjView, 1);
    layout->addWidget(new DHorizontalLine(this));
    layout->addLayout(bottomLayout);

    // TODO: run and report
    runBtn->setVisible(false);
    reportBtn->setVisible(false);

    return widget;
}

void SmartUTWidget::fillProjectView(const QString &workspace, const QStringList &fileList)
{
    prjView->clear();
    auto setting = SmartUTManager::instance()->utSetting();
    const auto &target = settingDlg->targetLocation();
    const auto &nameFormat = setting->value(kGeneralGroup, kNameFormat).toString();

    QSet<QString> utFileCache;
    std::vector<std::unique_ptr<FileNode>> fileNodes;
    for (const auto &f : fileList) {
        if (!Utils::isCppFile(f) && !Utils::isCMakeFile(f))
            continue;

        const auto &utFile = Utils::createUTFile(workspace, f, target, nameFormat);
        if (utFileCache.contains(utFile))
            continue;

        const auto &relatedFiles = Utils::relateFileList(f);
        if (relatedFiles.isEmpty())
            continue;

        auto fileNode = std::make_unique<FileNode>(utFile);
        fileNode->setSourceFiles(relatedFiles);
        utFileCache.insert(utFile);
        fileNodes.emplace_back(std::move(fileNode));
    }

    if (fileNodes.empty()) {
        mainWidget->setCurrentIndex(0);
        return;
    } else {
        mainWidget->setCurrentIndex(1);
    }

    ProjectNode *prjNode = new ProjectNode(workspace);
    prjNode->addNestedNodes(std::move(fileNodes), workspace);
    prjView->setRootProjectNode(prjNode);
}

void SmartUTWidget::createUTFiles()
{
    SmartUTManager::instance()->generateUTFiles(modelCB->currentText(), prjView->rootItem());
}

void SmartUTWidget::createUTFiles(NodeItem *item)
{
    SmartUTManager::instance()->generateUTFiles(modelCB->currentText(), item);
}

void SmartUTWidget::updateModelList()
{
    const auto model = modelCB->currentText();
    modelCB->clear();
    modelCB->addItems(SmartUTManager::instance()->modelList());
    modelCB->setCurrentText(model);
}

void SmartUTWidget::updateItemState(NodeItem *item)
{
    auto updateBtn = [this](bool isGenerating) {
        generateBtn->setVisible(!isGenerating);
        stopBtn->setVisible(isGenerating);
    };
    
    prjView->updateItem(item);
    if (item->state == Generating || item->state == Waiting) {
        updateBtn(true);
    } else if (Utils::checkAnyState(prjView->rootItem(), Generating)) {
        updateBtn(true);
    } else {
        updateBtn(false);
    }
}
