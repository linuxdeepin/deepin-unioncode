// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentdisplaywidget.h"
#include "recentlistview.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/project/projectgenerator.h"
#include "common/settings/settings.h"

#include <DLabel>
#include <DFrame>
#include <DStyle>
#include <DPushButton>
#include <DToolButton>
#include <DFileDialog>
#include <DStandardItem>
#include <DDialog>

#include <QImageReader>
#include <QList>
#include <QDir>
#include <QStandardItemModel>
#include <QListView>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QComboBox>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

constexpr char kRecentGroup[] { "Recent" };

static RecentDisplayWidget *ins { nullptr };

class RecentDisplayWidgetPrivate
{
    friend class RecentDisplayWidget;
    QHBoxLayout *hLayout { nullptr };
    QVBoxLayout *vLayoutDoc { nullptr };
    QVBoxLayout *vLayoutPro { nullptr };
    DWidget *recentOpen { nullptr };
    RecentProjectView *proView { nullptr };
    RecentDocemntView *docView { nullptr };
    QList<ItemListView *> viewList;
    DLabel *proLabel { nullptr };
    DToolButton *proClear { nullptr };
    DDialog *clearProConfirm { nullptr };

    DLabel *docLabel { nullptr };
    DToolButton *docClear { nullptr };
    DDialog *clearDocConfirm { nullptr };

    DFrame *navFrame { nullptr };
    DFrame *docFrame { nullptr };
    DFrame *proFrame { nullptr };
    DPushButton *btnOpenFile { nullptr };
    DPushButton *btnOpenProject { nullptr };
    DPushButton *btnNewFileOrPro { nullptr };
    DLabel *nullRecentText { nullptr };

    Settings recentSettings;
};

RecentDisplayWidget::RecentDisplayWidget(DWidget *parent)
    : DWidget(parent), d(new RecentDisplayWidgetPrivate())
{
    initializeUi();
    initConnect();
    initData();
}

RecentDisplayWidget::~RecentDisplayWidget()
{
    d->recentSettings.setValue(kRecentGroup, d->proView->configKey(), d->proView->itemList());
    d->recentSettings.setValue(kRecentGroup, d->docView->configKey(), d->docView->itemList());

    delete d;
}

RecentDisplayWidget *RecentDisplayWidget::instance()
{
    if (!ins)
        ins = new RecentDisplayWidget;
    return ins;
}

void RecentDisplayWidget::addDocument(const QString &filePath)
{
    auto itemList = d->docView->itemList();
    if (itemList.contains(filePath))
        itemList.removeOne(filePath);
    itemList.prepend(filePath);

    d->docView->clear();
    d->docView->setItemList(itemList);
}

void RecentDisplayWidget::addProject(const QString &kitName,
                                     const QString &language,
                                     const QString &workspace)
{
    QVariantMap item;
    item.insert(kKitName, kitName);
    item.insert(kLanguage, language);
    item.insert(kWorkspace, workspace);

    auto itemList = d->proView->itemList();
    if (itemList.contains(item))
        itemList.removeOne(item);
    itemList.prepend(item);

    d->proView->clear();
    d->proView->setItemList(itemList);
}

void RecentDisplayWidget::doDoubleClickedProject(const QModelIndex &index)
{
    QString kitName = index.data(RecentProjectView::KitNameRole).toString();
    QString language = index.data(RecentProjectView::LanguageRole).toString();
    QString workspace = index.data(RecentProjectView::WorkspaceRole).toString();
    // "kitName", "language", "workspace"
    project.openProject(kitName, language, workspace);
    RecentDisplayWidget::addProject(kitName, language, workspace);
}

void RecentDisplayWidget::doDoubleCliekedDocument(const QModelIndex &index)
{
    QString filePath = index.data(Qt::DisplayRole).toString();
    RecentDisplayWidget::addDocument(filePath);
    editor.openFile(QString(), filePath);
}

void RecentDisplayWidget::btnOpenFileClicked()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getOpenFileName(nullptr, tr("Open Document"), dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;
    recent.saveOpenedFile(filePath);
    editor.openFile(QString(), filePath);
}

void RecentDisplayWidget::btnOpenProjectClicked()
{
    auto projectService = dpfGetService(ProjectService);
    projectService->openProject();
}

void RecentDisplayWidget::btnNewFileOrProClicked()
{
    projectTemplate.newWizard();
}

bool RecentDisplayWidget::isProAndDocNull()
{
    return d->proView->itemList().isEmpty() && d->docView->itemList().isEmpty();
}

QVariantMap RecentDisplayWidget::parseProjectInfo(const QJsonObject &obj)
{
    if (obj.keys().size() != 1)
        return {};

    QString file = obj.keys()[0];
    if (file.isEmpty())
        return {};

    QVariantMap map;
    auto propertyObj = obj.value(file).toObject();
    map.insert(kKitName, propertyObj["KitName"].toString());
    map.insert(kLanguage, propertyObj["Language"].toString());
    map.insert(kWorkspace, propertyObj["Workspace"].toString());

    return map;
}

void RecentDisplayWidget::clearProList()
{
    d->proView->clear();
    d->recentSettings.removeGroup(d->proView->configKey());

    if (isProAndDocNull()) {
        d->nullRecentText->setVisible(true);
        d->recentOpen->setVisible(false);
    }
}

void RecentDisplayWidget::clearDocList()
{
    d->docView->clear();
    d->recentSettings.removeGroup(d->docView->configKey());

    if (isProAndDocNull()) {
        d->nullRecentText->setVisible(true);
        d->recentOpen->setVisible(false);
    }
}

void RecentDisplayWidget::initializeUi()
{
    d->navFrame = new DFrame();
    d->docFrame = new DFrame();
    d->proFrame = new DFrame();

    d->navFrame->setLineWidth(0);
    d->navFrame->setContentsMargins(0, 0, 0, 0);
    DStyle::setFrameRadius(d->navFrame, 0);
    QVBoxLayout *vLayoutNav = new QVBoxLayout();
    QLabel *recentLogo = new QLabel();
    QPixmap logo = QIcon(":/recent/images/recentLogo.png").pixmap(200, 143);
    logo.setDevicePixelRatio(recentLogo->devicePixelRatioF());
    recentLogo->setPixmap(logo);

    d->btnOpenFile = new DPushButton(tr("Open File"));
    d->btnOpenProject = new DPushButton(tr("Open Project"));
    d->btnNewFileOrPro = new DPushButton(tr("New File or Project"));
    d->nullRecentText = new DLabel(tr("No Project"));
    d->nullRecentText->setAlignment(Qt::AlignCenter);
    vLayoutNav->setContentsMargins(60, 0, 60, 0);
    vLayoutNav->setSpacing(20);
    vLayoutNav->setAlignment(Qt::AlignCenter);
    vLayoutNav->addWidget(recentLogo);
    vLayoutNav->addWidget(d->nullRecentText);
    vLayoutNav->addWidget(d->btnOpenFile);
    vLayoutNav->addWidget(d->btnOpenProject);
    vLayoutNav->addWidget(d->btnNewFileOrPro);
    d->navFrame->setLayout(vLayoutNav);

    DLabel *recentTitle = new DLabel(tr("Recent Open"));
    recentTitle->setForegroundRole(QPalette::BrightText);
    recentTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(recentTitle, DFontSizeManager::T4, QFont::Medium);

    //recent open document
    d->docFrame->setLineWidth(0);
    DStyle::setFrameRadius(d->docFrame, 0);
    d->docView = new RecentDocemntView(this);

    d->docLabel = new DLabel(tr("Documents"));
    d->docLabel->setForegroundRole(QPalette::BrightText);
    d->docLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->docLabel->setContentsMargins(10, 5, 0, 10);

    d->docClear = new DToolButton(this);
    d->docClear->setIcon(QIcon::fromTheme("ide_recent_delete"));
    d->docClear->setToolTip(tr("clear all"));

    d->clearDocConfirm = new DDialog(this);
    d->clearDocConfirm->setIcon(QIcon::fromTheme("dialog-warning"));
    d->clearDocConfirm->setMessage(tr("Confirm to clear the record of the opened file?"));
    d->clearDocConfirm->insertButton(0, tr("Cancel", "button"));
    d->clearDocConfirm->insertButton(1, tr("Delete", "button"), true, DDialog::ButtonWarning);

    QHBoxLayout *docHlayout = new QHBoxLayout;
    docHlayout->addWidget(d->docLabel);
    docHlayout->addWidget(d->docClear);

    DFontSizeManager::instance()->bind(d->docLabel, DFontSizeManager::T4, QFont::Medium);
    d->vLayoutDoc = new QVBoxLayout();
    d->vLayoutDoc->setContentsMargins(10, 10, 10, 10);
    d->vLayoutDoc->addLayout(docHlayout);
    d->vLayoutDoc->setSpacing(0);
    d->vLayoutDoc->addWidget(d->docView);
    d->docFrame->setLayout(d->vLayoutDoc);

    //recent open projects
    d->proFrame->setLineWidth(0);
    DStyle::setFrameRadius(d->proFrame, 0);
    d->proView = new RecentProjectView(this);

    d->proLabel = new DLabel(tr("Projects"), this);
    d->proLabel->setForegroundRole(QPalette::BrightText);
    d->proLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->proLabel->setContentsMargins(10, 5, 0, 10);

    d->proClear = new DToolButton(this);
    d->proClear->setIcon(QIcon::fromTheme("ide_recent_delete"));
    d->proClear->setToolTip(tr("clear all"));

    d->clearProConfirm = new DDialog(this);
    d->clearProConfirm->setIcon(QIcon::fromTheme("dialog-warning"));
    d->clearProConfirm->setMessage(tr("Confirm to clear the record of the opened project?"));
    d->clearProConfirm->insertButton(0, tr("Cancel", "button"));
    d->clearProConfirm->insertButton(1, tr("Delete", "button"), true, DDialog::ButtonWarning);

    QHBoxLayout *proHlayout = new QHBoxLayout;
    proHlayout->addWidget(d->proLabel);
    proHlayout->addWidget(d->proClear);

    DFontSizeManager::instance()->bind(d->proLabel, DFontSizeManager::T4, QFont::Medium);
    d->vLayoutPro = new QVBoxLayout();
    d->vLayoutPro->setContentsMargins(10, 10, 10, 10);
    d->vLayoutPro->addLayout(proHlayout);
    d->vLayoutPro->setSpacing(0);
    d->vLayoutPro->addWidget(d->proView);
    d->proFrame->setLayout(d->vLayoutPro);

    QHBoxLayout *proAndDocLayout = new QHBoxLayout();
    proAndDocLayout->addWidget(d->proFrame);
    proAndDocLayout->setSpacing(2);
    proAndDocLayout->addWidget(d->docFrame);

    d->recentOpen = new DWidget(this);
    QVBoxLayout *recentNavLayout = new QVBoxLayout(d->recentOpen);
    recentNavLayout->setContentsMargins(15, 0, 25, 20);
    recentNavLayout->addSpacing(25);
    recentNavLayout->setAlignment(Qt::AlignTop);
    recentNavLayout->addWidget(recentTitle);
    recentNavLayout->addSpacing(5);
    recentNavLayout->addLayout(proAndDocLayout);

    d->hLayout = new QHBoxLayout(this);
    d->hLayout->setContentsMargins(0, 0, 0, 0);

    d->hLayout->addWidget(d->navFrame);
    d->hLayout->addWidget(d->recentOpen);

    if (!isProAndDocNull()) {
        d->nullRecentText->setVisible(false);
        d->recentOpen->setVisible(true);
    }
}

void RecentDisplayWidget::initConnect()
{
    QObject::connect(d->proView, &QListView::doubleClicked,
                     this, &RecentDisplayWidget::doDoubleClickedProject,
                     Qt::UniqueConnection);

    QObject::connect(d->docView, &QListView::doubleClicked,
                     this, &RecentDisplayWidget::doDoubleCliekedDocument,
                     Qt::UniqueConnection);

    QObject::connect(d->btnOpenFile, &DPushButton::clicked,
                     this, &RecentDisplayWidget::btnOpenFileClicked,
                     Qt::UniqueConnection);

    QObject::connect(d->btnOpenProject, &DPushButton::clicked,
                     this, &RecentDisplayWidget::btnOpenProjectClicked,
                     Qt::UniqueConnection);

    QObject::connect(d->btnNewFileOrPro, &DPushButton::clicked,
                     this, &RecentDisplayWidget::btnNewFileOrProClicked,
                     Qt::UniqueConnection);

    QObject::connect(d->docClear, &DPushButton::clicked,
                     d->clearDocConfirm, &DDialog::exec,
                     Qt::UniqueConnection);

    QObject::connect(d->proClear, &DPushButton::clicked,
                     d->clearProConfirm, &DDialog::exec,
                     Qt::UniqueConnection);

    QObject::connect(d->clearDocConfirm, &DDialog::buttonClicked, this, [=](int index) {
        if (index == 0)
            d->clearDocConfirm->reject();
        else if (index == 1)
            clearDocList();
    });

    QObject::connect(d->clearProConfirm, &DDialog::buttonClicked, this, [=](int index) {
        if (index == 0)
            d->clearProConfirm->reject();
        else if (index == 1)
            clearProList();
    });
}

[[deprecated("-------------存在兼容代码需要删除")]] void RecentDisplayWidget::initData()
{
    const QString oldCfgFile = CustomPaths::user(CustomPaths::Configures) + QDir::separator() + "recent.support";
    const QString newCfgFile = CustomPaths::user(CustomPaths::Configures) + QDir::separator() + "recent.json";
    d->recentSettings.load("", newCfgFile);

    QFile file(oldCfgFile);
    if (file.open(QIODevice::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        file.remove();

        QVariantList prjList;
        QJsonArray prjArray = doc.object().value(d->proView->configKey()).toArray();
        for (const auto &prj : prjArray) {
            auto prjObj = prj.toObject();
            const auto &info = parseProjectInfo(prjObj);
            if (!info.isEmpty())
                prjList << info;
        }
        d->recentSettings.setValue(kRecentGroup, d->proView->configKey(), prjList);

        QVariantList docList;
        QJsonArray docArray = doc.object().value(d->docView->configKey()).toArray();
        for (const auto &doc : docArray) {
            auto filePath = doc.toString();
            if (!filePath.isEmpty())
                docList << filePath;
        }
        d->recentSettings.setValue(kRecentGroup, d->docView->configKey(), docList);
    }

    d->proView->setItemList(d->recentSettings.value(kRecentGroup, d->proView->configKey()).toList());
    d->docView->setItemList(d->recentSettings.value(kRecentGroup, d->docView->configKey()).toList());
}

void RecentDisplayWidget::showEvent(QShowEvent *event)
{
    if (!isProAndDocNull()) {
        d->nullRecentText->setVisible(false);
        d->recentOpen->setVisible(true);
    } else {
        d->nullRecentText->setVisible(true);
        d->recentOpen->setVisible(false);
    }
}
