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
#include <QApplication>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

constexpr char kRecentGroup[] { "Recent" };
constexpr char kProjects[] { "Projects" };
constexpr char kDocuments[] { "Documents" };

static RecentDisplayWidget *ins { nullptr };

class RecentDisplayWidgetPrivate
{
public:
    explicit RecentDisplayWidgetPrivate(RecentDisplayWidget *qq);

    void createRecentWidget();
    void createSessionWidget();

public:
    RecentDisplayWidget *q;

    DWidget *recentOpen { nullptr };
    RecentListView *recentListView { nullptr };
    SessionItemListWidget *sessionListWidget { nullptr };
    DToolButton *recentClearBtn { nullptr };
    DDialog *clearRecentConfirm { nullptr };
    DToolButton *sessionSetBtn { nullptr };
    DFrame *navFrame { nullptr };
    DFrame *recentFrame { nullptr };
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

void RecentDisplayWidgetPrivate::createRecentWidget()
{
    recentFrame = new DFrame(q);
    recentFrame->setLineWidth(0);
    DStyle::setFrameRadius(recentFrame, 0);
    recentListView = new RecentListView(q);

    DLabel *titleLabel = new DLabel(RecentDisplayWidget::tr("Projects And Documents"), q);
    titleLabel->setForegroundRole(QPalette::BrightText);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleLabel->setContentsMargins(10, 5, 0, 10);

    recentClearBtn = new DToolButton(q);
    recentClearBtn->setIcon(QIcon::fromTheme("ide_recent_delete"));
    recentClearBtn->setToolTip(RecentDisplayWidget::tr("Clear All"));

    clearRecentConfirm = new DDialog(q);
    clearRecentConfirm->setIcon(QIcon::fromTheme("dialog-warning"));
    clearRecentConfirm->setMessage(RecentDisplayWidget::tr("Confirm to clear the record of the opened projects and documents?"));
    clearRecentConfirm->insertButton(0, RecentDisplayWidget::tr("Cancel", "button"));
    clearRecentConfirm->insertButton(1, RecentDisplayWidget::tr("Delete", "button"), true, DDialog::ButtonWarning);

    QHBoxLayout *headerlayout = new QHBoxLayout;
    headerlayout->addWidget(titleLabel);
    headerlayout->addWidget(recentClearBtn);

    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T4, QFont::Medium);
    QVBoxLayout *vLayout = new QVBoxLayout(recentFrame);
    vLayout->setContentsMargins(10, 10, 10, 10);
    vLayout->setSpacing(0);
    vLayout->addLayout(headerlayout);
    vLayout->addWidget(recentListView);
}

void RecentDisplayWidgetPrivate::createSessionWidget()
{
    sessionFrame = new DFrame(q);
    sessionFrame->setLineWidth(0);
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
    if (d->recentListView->contains(filePath)) {
        auto itemList = d->recentListView->documentList();
        itemList.removeOne(filePath);
        itemList.prepend(filePath);
        d->recentListView->clearDocuments();
        d->recentListView->setItemList(itemList);
    } else {
        d->recentListView->prependItem(filePath);
    }
}

void RecentDisplayWidget::addProject(const QString &kitName,
                                     const QString &language,
                                     const QString &workspace)
{
    QVariantMap item;
    item.insert(kKitName, kitName);
    item.insert(kLanguage, language);
    item.insert(kWorkspace, workspace);

    if (d->recentListView->contains(workspace)) {
        auto itemList = d->recentListView->projectList();
        itemList.removeOne(item);
        itemList.prepend(item);
        d->recentListView->clearProjects();
        d->recentListView->setItemList(itemList);
    } else {
        d->recentListView->prependItem(item);
    }
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

void RecentDisplayWidget::doDoubleClicked(const QModelIndex &index)
{
    if (index.data(RecentListView::IsProject).toBool()) {
        QString kitName = index.data(RecentListView::KitNameRole).toString();
        QString language = index.data(RecentListView::LanguageRole).toString();
        QString workspace = index.data(RecentListView::WorkspaceRole).toString();
        // "kitName", "language", "workspace"
        project.openProject(kitName, language, workspace);
        RecentDisplayWidget::addProject(kitName, language, workspace);
    } else {
        QString filePath = index.data(Qt::DisplayRole).toString();
        RecentDisplayWidget::addDocument(filePath);
        editor.openFile(QString(), filePath);
    }
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
    return d->recentListView->isEmpty();
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

void RecentDisplayWidget::clearRecent()
{
    d->recentListView->clearAll();

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

    d->createRecentWidget();
    d->createSessionWidget();

    QHBoxLayout *proAndDocLayout = new QHBoxLayout();
    proAndDocLayout->addWidget(d->recentFrame);
    proAndDocLayout->setSpacing(2);
    proAndDocLayout->addWidget(d->sessionFrame);

    d->recentOpen = new DWidget(this);
    QVBoxLayout *recentNavLayout = new QVBoxLayout(d->recentOpen);
    recentNavLayout->setContentsMargins(15, 0, 25, 20);
    recentNavLayout->addSpacing(25);
    recentNavLayout->setAlignment(Qt::AlignTop);
    recentNavLayout->addWidget(recentTitle);
    recentNavLayout->addSpacing(5);
    recentNavLayout->addLayout(proAndDocLayout);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(d->navFrame);
    hLayout->addWidget(d->recentOpen);

    if (!isProAndDocNull()) {
        d->nullRecentText->setVisible(false);
        d->recentOpen->setVisible(true);
    }
}

void RecentDisplayWidget::initConnect()
{
    QObject::connect(d->recentListView, &QListView::doubleClicked,
                     this, &RecentDisplayWidget::doDoubleClicked,
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

    QObject::connect(d->recentClearBtn, &DPushButton::clicked,
                     d->clearRecentConfirm, &DDialog::exec,
                     Qt::UniqueConnection);

    QObject::connect(d->clearRecentConfirm, &DDialog::buttonClicked, this, [=](int index) {
        if (index == 0)
            d->clearRecentConfirm->reject();
        else if (index == 1)
            clearRecent();
    });
    QObject::connect(d->sessionSetBtn, &DToolButton::clicked, this, [this] {
        d->sessionSrv->showSessionManager();
    });
    QObject::connect(qApp, &QApplication::aboutToQuit, this, [this] {
        d->recentSettings.setValue(kRecentGroup, kProjects, d->recentListView->projectList());
        d->recentSettings.setValue(kRecentGroup, kDocuments, d->recentListView->documentList());
        d->recentSettings.sync();
    });
}

Q_DECL_DEPRECATED_X("-------------存在兼容代码需要删除")
void RecentDisplayWidget::initData()
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
        QJsonArray prjArray = doc.object().value(kProjects).toArray();
        for (const auto &prj : prjArray) {
            auto prjObj = prj.toObject();
            const auto &info = parseProjectInfo(prjObj);
            if (!info.isEmpty())
                prjList << info;
        }
        d->recentSettings.setValue(kRecentGroup, kProjects, prjList);

        QVariantList docList;
        QJsonArray docArray = doc.object().value(kDocuments).toArray();
        for (const auto &doc : docArray) {
            auto filePath = doc.toString();
            if (!filePath.isEmpty())
                docList << filePath;
        }
        d->recentSettings.setValue(kRecentGroup, kDocuments, docList);
    }

    d->recentListView->setItemList(d->recentSettings.value(kRecentGroup, kProjects).toList());
    d->recentListView->setItemList(d->recentSettings.value(kRecentGroup, kDocuments).toList());
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
