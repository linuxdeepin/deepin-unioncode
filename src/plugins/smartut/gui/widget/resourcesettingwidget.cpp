// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resourcesettingwidget.h"
#include "manager/smartutmanager.h"
#include "utils/utils.h"

#include <DStyle>
#include <DLabel>

#include <QFileInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include <QEvent>

DWIDGET_USE_NAMESPACE

ResourceSettingWidget::ResourceSettingWidget(QWidget *parent)
    : DFrame(parent)
{
    initUI();
    initConnection();
}

void ResourceSettingWidget::initUI()
{
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setSpacing(10);

    projectCB = new DComboBox(this);
    prjView = new ProjectTreeView(ProjectTreeView::Project, this);
    prjView->viewport()->installEventFilter(this);

    targetLocationEdit = new DLineEdit(this);
    targetSelBtn = new DSuggestButton(this);
    targetSelBtn->setIconSize({ 16, 16 });
    targetSelBtn->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));

    mainLayout->addWidget(projectCB, 0, 0, 1, 3);
    mainLayout->addWidget(new DLabel(tr("Source Files"), this), 1, 0);
    mainLayout->addWidget(prjView, 1, 1, 1, 2);
    mainLayout->addWidget(new DLabel(tr("Target Location"), this), 2, 0);
    mainLayout->addWidget(targetLocationEdit, 2, 1);
    mainLayout->addWidget(targetSelBtn, 2, 2);
}

void ResourceSettingWidget::initConnection()
{
    connect(projectCB, &DComboBox::currentTextChanged, this, &ResourceSettingWidget::handleProjectChanged);
    connect(targetSelBtn, &DSuggestButton::clicked, this, &ResourceSettingWidget::handleSelectLocation);
    connect(targetLocationEdit, &DLineEdit::textChanged, this,
            [this] {
                if (targetLocationEdit->isAlert())
                    targetLocationEdit->setAlert(false);
            });
}

void ResourceSettingWidget::updateSettings()
{
    const auto &infoList = SmartUTManager::instance()->projectList();
    QStringList projectList;
    std::transform(infoList.cbegin(), infoList.cend(), std::back_inserter(projectList),
                   [](const dpfservice::ProjectInfo &info) {
                       return QFileInfo(info.workspaceFolder()).baseName();
                   });

    for (int i = projectCB->count() - 1; i >= 0; --i) {
        if (!projectList.contains(projectCB->itemText(i)))
            projectCB->removeItem(i);
    }

    for (const auto &info : infoList) {
        const auto &itemText = QFileInfo(info.workspaceFolder()).baseName();
        if (projectCB->findText(itemText) == -1)
            projectCB->addItem(itemText, qVariantFromValue(info));
    }
}

QStringList ResourceSettingWidget::selectedFileList(NodeItem *item) const
{
    QStringList selList;
    for (int i = 0; i < item->rowCount(); ++i) {
        NodeItem *nodeItem = dynamic_cast<NodeItem *>(item->child(i));
        if (!nodeItem)
            continue;

        bool isChecked = nodeItem->checkState() != Qt::Unchecked;
        if (!isChecked)
            continue;

        if (!nodeItem->itemNode->isFileNodeType())
            selList << selectedFileList(nodeItem);
        else
            selList << nodeItem->itemNode->filePath();
    }

    return selList;
}

void ResourceSettingWidget::handleProjectChanged()
{
    prjView->clear();
    const auto &prjInfo = projectCB->currentData().value<dpfservice::ProjectInfo>();
    if (prjInfo.isEmpty())
        return;

    auto prjNode = Utils::createProjectNode(prjInfo);
    prjView->setRootProjectNode(prjNode);

    QString target = targetLocationEdit->text();
    if (target.isEmpty()) {
        QString path = prjInfo.workspaceFolder() + QDir::separator() + "test";
        targetLocationEdit->setText(path);
    } else if (prjInfoCache.isVaild() && target.startsWith(prjInfoCache.workspaceFolder())) {
        const auto &subFolder = target.mid(prjInfoCache.workspaceFolder().size());
        targetLocationEdit->setText(prjInfo.workspaceFolder() + subFolder);
    }

    prjInfoCache = prjInfo;
}

void ResourceSettingWidget::handleSelectLocation()
{
    const auto &prjInfo = projectCB->currentData().value<dpfservice::ProjectInfo>();
    const auto &path = QFileDialog::getExistingDirectory(this, tr("Select target location"),
                                                         prjInfo.isEmpty()
                                                                 ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                                                 : prjInfo.workspaceFolder());
    if (!path.isEmpty())
        targetLocationEdit->setText(path);
}

bool ResourceSettingWidget::apply()
{
    const auto &target = targetLocationEdit->text();
    if (target.isEmpty() || !Utils::isValidPath(target) || !target.startsWith(selectedProject())) {
        targetLocationEdit->setAlert(true);
        targetLocationEdit->showAlertMessage(tr("Please input a valid path with the selected project"));
        targetLocationEdit->setFocus();
        targetLocationEdit->lineEdit()->selectAll();
        return false;
    }

    return true;
}

QStringList ResourceSettingWidget::selectedFileList() const
{
    NodeItem *rootItem = prjView->rootItem();
    if (!rootItem)
        return {};

    return selectedFileList(rootItem);
}

QString ResourceSettingWidget::selectedProject() const
{
    const auto &prjInfo = projectCB->currentData().value<dpfservice::ProjectInfo>();
    return prjInfo.workspaceFolder();
}

QString ResourceSettingWidget::targetLocation() const
{
    return targetLocationEdit->text();
}

bool ResourceSettingWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (prjView && obj == prjView->viewport() && e->type() == QEvent::Paint) {
        QPainter painter(prjView->viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        auto p = prjView->viewport()->palette();
        painter.setPen(Qt::NoPen);
        painter.setBrush(p.brush(QPalette::Active, QPalette::AlternateBase));

        QPainterPath path;
        path.addRoundedRect(prjView->viewport()->rect(), 8, 8);
        painter.drawPath(path);
    }

    return DFrame::eventFilter(obj, e);
}
