// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentdisplaywidget.h"
#include "recentlistview.h"
#include "sessionitemwidget.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/project/projectgenerator.h"
#include "services/session/sessionservice.h"
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
public:
    explicit RecentDisplayWidgetPrivate(RecentDisplayWidget *qq);

    void createProjectWidget();
    void createDocumentWidget();
    void createSessionWidget();

public:
    RecentDisplayWidget *q;

    QHBoxLayout *hLayout { nullptr };
    QVBoxLayout *vLayoutDoc { nullptr };
    QVBoxLayout *vLayoutPro { nullptr };
    DWidget *recentOpen { nullptr };
    RecentProjectView *proView { nullptr };
    RecentDocemntView *docView { nullptr };
    SessionItemListWidget *sessionListWidget { nullptr };
    QList<ItemListView *> viewList;
    DLabel *proLabel { nullptr };
    DToolButton *proClear { nullptr };
    DDialog *clearProConfirm { nullptr };

    DLabel *docLabel { nullptr };
    DToolButton *docClear { nullptr };
    DDialog *clearDocConfirm { nullptr };

    DToolButton *sessionSetBtn { nullptr };

    DFrame *navFrame { nullptr };
    DFrame *docFrame { nullptr };
    DFrame *proFrame { nullptr };
    DFrame *sessionFrame { nullptr };
    DPushButton *btnOpenFile { nullptr };
    DPushButton *btnOpenProject { nullptr };
    DPushButton *btnNewFileOrPro { nullptr };
    DLabel *nullRecentText { nullptr };

    Settings recentSettings;
    SessionService *sessionSrv { nullptr };
};

RecentDisplayWidgetPrivate::RecentDisplayWidgetPrivate(RecentDisplayWidget *qq)
    : q(qq)
{
    sessionSrv = dpfGetService(SessionService);
}

void RecentDisplayWidgetPrivate::createProjectWidget()
{
    proFrame = new DFrame(q);
    proFrame->setLineWidth(0);
    DStyle::setFrameRadius(proFrame, 0);
    proView = new RecentProjectView(q);

    proLabel = new DLabel(RecentDisplayWidget::tr("Projects"), q);
    proLabel->setForegroundRole(QPalette::BrightText);
    proLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    proLabel->setContentsMargins(10, 5, 0, 10);

    proClear = new DToolButton(q);
    proClear->setIcon(QIcon::fromTheme("ide_recent_delete"));
    proClear->setToolTip(RecentDisplayWidget::tr("clear all"));

    clearProConfirm = new DDialog(q);
    clearProConfirm->setIcon(QIcon::fromTheme("dialog-warning"));
    clearProConfirm->setMessage(RecentDisplayWidget::tr("Confirm to clear the record of the opened project?"));
    clearProConfirm->insertButton(0, RecentDisplayWidget::tr("Cancel", "button"));
    clearProConfirm->insertButton(1, RecentDisplayWidget::tr("Delete", "button"), true, DDialog::ButtonWarning);

    QHBoxLayout *proHlayout = new QHBoxLayout;
    proHlayout->addWidget(proLabel);
    proHlayout->addWidget(proClear);

    DFontSizeManager::instance()->bind(proLabel, DFontSizeManager::T4, QFont::Medium);
    vLayoutPro = new QVBoxLayout();
    vLayoutPro->setContentsMargins(10, 10, 10, 10);
    vLayoutPro->addLayout(proHlayout);
    vLayoutPro->setSpacing(0);
    vLayoutPro->addWidget(proView);
    proFrame->setLayout(vLayoutPro);
}

void RecentDisplayWidgetPrivate::createDocumentWidget()
{
    docFrame = new DFrame();
    docFrame->setLineWidth(0);
    DStyle::setFrameRadius(docFrame, 0);
    docView = new RecentDocemntView(q);

    docLabel = new DLabel(RecentDisplayWidget::tr("Documents"));
    docLabel->setForegroundRole(QPalette::BrightText);
    docLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    docLabel->setContentsMargins(10, 5, 0, 10);

    docClear = new DToolButton(q);
    docClear->setIcon(QIcon::fromTheme("ide_recent_delete"));
    docClear->setToolTip(RecentDisplayWidget::tr("clear all"));

    clearDocConfirm = new DDialog(q);
    clearDocConfirm->setIcon(QIcon::fromTheme("dialog-warning"));
    clearDocConfirm->setMessage(RecentDisplayWidget::tr("Confirm to clear the record of the opened file?"));
    clearDocConfirm->insertButton(0, RecentDisplayWidget::tr("Cancel", "button"));
    clearDocConfirm->insertButton(1, RecentDisplayWidget::tr("Delete", "button"), true, DDialog::ButtonWarning);

    QHBoxLayout *docHlayout = new QHBoxLayout;
    docHlayout->addWidget(docLabel);
    docHlayout->addWidget(docClear);

    DFontSizeManager::instance()->bind(docLabel, DFontSizeManager::T4, QFont::Medium);
    vLayoutDoc = new QVBoxLayout();
    vLayoutDoc->setContentsMargins(10, 10, 10, 10);
    vLayoutDoc->addLayout(docHlayout);
    vLayoutDoc->setSpacing(0);
    vLayoutDoc->addWidget(docView);
    docFrame->setLayout(vLayoutDoc);
}

void RecentDisplayWidgetPrivate::createSessionWidget()
{
    sessionFrame = new DFrame(q);
    sessionFrame->setLineWidth(0);
    sessionFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    DStyle::setFrameRadius(sessionFrame, 0);
    sessionListWidget = new SessionItemListWidget(q);

    DLabel *sessionLabel = new DLabel(RecentDisplayWidget::tr("Session"), q);
    sessionLabel->setForegroundRole(QPalette::BrightText);
    sessionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sessionLabel->setContentsMargins(10, 5, 0, 10);
    DFontSizeManager::instance()->bind(sessionLabel, DFontSizeManager::T4, QFont::Medium);

    sessionSetBtn = new DToolButton(q);
    sessionSetBtn->setIcon(QIcon::fromTheme("uc_settings"));
    sessionSetBtn->setToolTip(RecentDisplayWidget::tr("session manager"));

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(sessionLabel);
    headerLayout->addWidget(sessionSetBtn);

    QVBoxLayout *vLayout = new QVBoxLayout(sessionFrame);
    vLayout->setContentsMargins(10, 10, 10, 10);
    vLayout->setSpacing(0);
    vLayout->addLayout(headerLayout);
    vLayout->addWidget(sessionListWidget);
}

RecentDisplayWidget::RecentDisplayWidget(DWidget *parent)
    : DWidget(parent),
      d(new RecentDisplayWidgetPrivate(this))
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

void RecentDisplayWidget::addSession(const QString &session)
{
    d->sessionListWidget->addSessionList({ session });
}

void RecentDisplayWidget::removeSession(const QString &session)
{
    d->sessionListWidget->removeSession(session);
}

void RecentDisplayWidget::updateSessions()
{
    d->sessionListWidget->updateSessions();
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

    d->createDocumentWidget();
    d->createProjectWidget();
    d->createSessionWidget();

    QVBoxLayout *sessionDocLayout = new QVBoxLayout;
    sessionDocLayout->setSpacing(2);
    sessionDocLayout->addWidget(d->docFrame);
    sessionDocLayout->addWidget(d->sessionFrame);

    QHBoxLayout *proAndDocLayout = new QHBoxLayout();
    proAndDocLayout->addWidget(d->proFrame);
    proAndDocLayout->setSpacing(2);
    proAndDocLayout->addLayout(sessionDocLayout);

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
    QObject::connect(d->sessionSetBtn, &DToolButton::clicked, this, [this] {
        d->sessionSrv->showSessionManager();
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
    d->sessionListWidget->addSessionList(d->sessionSrv->sessionList());
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
