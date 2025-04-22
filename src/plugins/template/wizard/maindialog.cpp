// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maindialog.h"
#include "detailwidget.h"
#include "services/language/languageservice.h"

#include <DLabel>
#include <DWidget>
#include <DTreeView>
#include <DMessageBox>
#include <DPushButton>
#include <DTitlebar>
#include <DListWidget>
#include <DStackedWidget>
#include <DStandardItem>
#include <DSuggestButton>
#include <DButtonBox>
#include <DFrame>

#include <QStandardItemModel>
#include <QDebug>
#include <QHBoxLayout>
#include <QUuid>
#include <QDebug>

const int widthPerBtn = 93;

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class MainDialogPrivate
{
    friend class MainDialog;
    QMap<QString, DetailWidget *> detailWidgetMap;
    DStackedWidget *detailStackedWidget = nullptr;
    DStackedWidget *StackedWidget = nullptr;
    DFrame *blankWidget = nullptr;
};

MainDialog::MainDialog(DWidget *parent)
    : DAbstractDialog(parent), d(new MainDialogPrivate())
{
    setMinimumSize(611, 427);
    TemplateVector templateVec;
    TemplateParser::readTemplateConfig(templateVec);
    setupUI(templateVec);
}

MainDialog::~MainDialog()
{
    if (d)
        delete d;
}

void MainDialog::setupUI(TemplateVector &templateVec)
{
    DTitlebar *titleBar = new DTitlebar(this);
    titleBar->setMenuVisible(false);
    titleBar->setIcon(QIcon::fromTheme("ide"));

    DButtonBoxButton *newFileButton = new DButtonBoxButton(QObject::tr("New File"), this);
    DButtonBoxButton *newProjectButton = new DButtonBoxButton(QObject::tr("New Project"), this);
    DButtonBox *btnbox = new DButtonBox(this);
    QList<DButtonBoxButton *> list { newFileButton, newProjectButton };
    btnbox->setButtonList(list, true);
    btnbox->setFixedWidth(widthPerBtn * btnbox->buttonList().size());
    titleBar->addWidget(btnbox);
    newFileButton->click();

    DFrame *detailFrame = new DFrame(this);
    QHBoxLayout *detailLayout = new QHBoxLayout();
    detailFrame->setLayout(detailLayout);
    detailFrame->setLineWidth(0);

    d->detailStackedWidget = new DStackedWidget(detailFrame);
    d->StackedWidget = new DStackedWidget(this);
    d->detailStackedWidget->setContentsMargins(0, 0, 0, 0);
    detailLayout->addWidget(d->detailStackedWidget);

    d->blankWidget = new DFrame(this);
    d->blankWidget->setLineWidth(0);
    d->detailStackedWidget->addWidget(d->blankWidget);
    d->StackedWidget->setLineWidth(0);

    DFrame *leftFrame = new DFrame(this);
    DTreeView *treeView = new DTreeView(leftFrame);
    QHBoxLayout *leftFrameLayout = new QHBoxLayout();
    treeView->setHeaderHidden(true);

    treeView->setEditTriggers(DTreeView::NoEditTriggers);
    treeView->setSelectionMode(DTreeView::SingleSelection);
    treeView->setSelectionBehavior(DTreeView::SelectRows);
    treeView->setSelectionMode(DTreeView::SingleSelection);
    treeView->setLineWidth(0);
    leftFrameLayout->addWidget(treeView);
    leftFrameLayout->setContentsMargins(10, 0, 10, 0);
    leftFrame->setLayout(leftFrameLayout);
    leftFrame->setLineWidth(0);
    leftFrame->setFixedWidth(164);
    DStyle::setFrameRadius(leftFrame, 0);

    //deafult new file显示
    QStandardItemModel *StandardModel = new QStandardItemModel();
    QStandardItem *rootItem = StandardModel->invisibleRootItem();

    auto iterTpl = templateVec.begin();

    QVector<TemplateCategory> tplVec = iterTpl->templateVec;
    for (auto iterCate = tplVec.begin(); iterCate != tplVec.end(); ++iterCate) {
        DStandardItem *typeItem = new DStandardItem(iterCate->type);
        rootItem->appendRow(typeItem);
        auto iterDetail = iterCate->templateVec.begin();
        for (; iterDetail != iterCate->templateVec.end(); ++iterDetail) {
            DStandardItem *detailItem = new DStandardItem(iterDetail->name);
            TemplateDetail detail;
            detail.name = iterDetail->name;
            detail.path = iterDetail->path;
            detail.leafNode = iterDetail->leafNode;
            detailItem->setData(QVariant::fromValue(detail), Qt::UserRole + 1);
            detailItem->setData(QVariant::fromValue(QUuid::createUuid().toString()), Qt::UserRole + 2);
            typeItem->appendRow(detailItem);
        }
    }
    treeView->setModel(StandardModel);
    treeView->expandAll();

    connect(btnbox, &DButtonBox::buttonClicked, this, [=](QAbstractButton *button) {
        d->detailStackedWidget->setCurrentWidget(d->blankWidget);
        if (button == newFileButton) {
            QStandardItemModel *StandardModel = new QStandardItemModel();
            QStandardItem *rootItem = StandardModel->invisibleRootItem();

            auto iterTpl = templateVec.begin();

            QVector<TemplateCategory> tplVec = iterTpl->templateVec;
            for (auto iterCate = tplVec.begin(); iterCate != tplVec.end(); ++iterCate) {
                DStandardItem *typeItem = new DStandardItem(iterCate->type);
                rootItem->appendRow(typeItem);
                auto iterDetail = iterCate->templateVec.begin();
                for (; iterDetail != iterCate->templateVec.end(); ++iterDetail) {
                    DStandardItem *detailItem = new DStandardItem(iterDetail->name);
                    TemplateDetail detail;
                    detail.name = iterDetail->name;
                    detail.path = iterDetail->path;
                    detail.leafNode = iterDetail->leafNode;

                    detailItem->setData(QVariant::fromValue(detail), Qt::UserRole + 1);
                    detailItem->setData(QVariant::fromValue(QUuid::createUuid().toString()), Qt::UserRole + 2);
                    typeItem->appendRow(detailItem);
                }
            }
            treeView->setModel(StandardModel);
            treeView->expandAll();
        }
        if (button == newProjectButton) {
            QStandardItemModel *StandardModel = new QStandardItemModel();
            QStandardItem *rootItem = StandardModel->invisibleRootItem();
            auto iterTpl = templateVec.begin() + 1;

            QVector<TemplateCategory> tplVec = iterTpl->templateVec;
            for (auto iterCate = tplVec.begin(); iterCate != tplVec.end(); ++iterCate) {

                DStandardItem *typeItem = new DStandardItem(iterCate->type);
                rootItem->appendRow(typeItem);

                auto iterDetail = iterCate->templateVec.begin();
                for (; iterDetail != iterCate->templateVec.end(); ++iterDetail) {
                    DStandardItem *detailItem = new DStandardItem(iterDetail->name);
                    TemplateDetail detail;
                    detail.name = iterDetail->name;
                    detail.path = iterDetail->path;
                    detail.leafNode = iterDetail->leafNode;

                    detailItem->setData(QVariant::fromValue(detail), Qt::UserRole + 1);
                    detailItem->setData(QVariant::fromValue(QUuid::createUuid().toString()), Qt::UserRole + 2);
                    typeItem->appendRow(detailItem);
                }
            }
            treeView->setModel(StandardModel);
            treeView->expandAll();
        }
    });

    connect(treeView, &DTreeView::clicked, [=]() {
        QModelIndex index = treeView->selectionModel()->currentIndex();
        if (!index.isValid()) {
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        QVariant varDetail = index.data(Qt::UserRole + 1);
        if (!varDetail.isValid()) {
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        TemplateDetail detail = varDetail.value<TemplateDetail>();
        if (!detail.leafNode) {
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        QVariant varUuid = index.data(Qt::UserRole + 2);
        if (!varUuid.isValid()) {
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        QString uuid = varUuid.value<QString>();
        if (d->detailWidgetMap.contains(uuid)) {
            DetailWidget *detailWidget = d->detailWidgetMap.value(uuid);
            if (detailWidget) {
                d->detailStackedWidget->setCurrentWidget(detailWidget);
            }
        } else {
            DetailWidget *detailWidget = new DetailWidget(detail.path, this);

            d->detailWidgetMap.insert(uuid, detailWidget);
            d->detailStackedWidget->addWidget(detailWidget);
            d->detailStackedWidget->setCurrentWidget(detailWidget);

            //关闭主窗口
            connect(detailWidget, &DetailWidget::closeSignal, this, [this]() {
                close();
            });
        }
    });

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(leftFrame);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(detailFrame);
    rightLayout->setContentsMargins(10, 10, 10, 10);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(leftLayout);
    hLayout->addLayout(rightLayout);

    hLayout->setSpacing(0);
    QVBoxLayout *vLayout = new QVBoxLayout();

    vLayout->addWidget(titleBar);
    vLayout->setSpacing(0);
    vLayout->addLayout(hLayout);
    vLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(vLayout);
}

void MainDialog::generate(const PojectGenParam &param)
{
    ProjectGenerate projectGen;
    PojectGenResult result;
    if (!projectGen.create(result, param)) {
        DMessageBox::critical(this, tr("Tip"), result.message);
        return;
    }

    if (param.type == Project) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            auto generator = service->create<LanguageGenerator>(result.kit);
            if (generator) {
                close();
                project.openProject(result.kit, result.language, result.projectPath);
            } else {
                DMessageBox::critical(this, tr("Tip"), tr("Can not find kit."));
            }
        }
    } else if (param.type == File) {
        close();
        editor.openFile(QString(), result.filePath);
    }
}
