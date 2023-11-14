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
#include <DTreeView>
#include <DTitlebar>
#include <DListWidget>

#include <QStackedWidget>
#include <QStandardItemModel>
#include <QDebug>
#include <QHBoxLayout>
#include <QUuid>
#include <QStackedWidget>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class MainDialogPrivate
{
    friend class MainDialog;
    QMap<QString, DetailWidget*> detailWidgetMap;
    QStackedWidget *detailStackedWidget = nullptr;
    DWidget *blankWidget = nullptr;
};

MainDialog::MainDialog(QWidget *parent)
    : DAbstractDialog(parent)
    , d(new MainDialogPrivate())
{
    setMinimumSize(850, 550);

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
    DTitlebar *titleBar = new DTitlebar();
    titleBar = new DTitlebar();
    titleBar->setMenuVisible(false);
    titleBar->setTitle(QString(tr("New File or Project")));

    DLabel *title =  new DLabel(tr("Choose a template:"));

    d->detailStackedWidget = new QStackedWidget();
    d->blankWidget = new DetailWidget(this);
    d->detailStackedWidget->addWidget(d->blankWidget);

    DTreeView * treeView =  new DTreeView();
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setSelectionBehavior(QAbstractItemView::SelectItems);

    QStandardItemModel *standardModel = new QStandardItemModel();
    standardModel->setHorizontalHeaderLabels(QStringList{tr("Templates")});

    QStandardItem * rootItem = standardModel->invisibleRootItem();

    for (auto iterTpl = templateVec.begin(); iterTpl != templateVec.end(); ++iterTpl) {
        QStandardItem *tpl = new QStandardItem(iterTpl->category);
        rootItem->appendRow(tpl);

        QVector<TemplateCategory> tplVec = iterTpl->templateVec;
        for (auto iterCate = tplVec.begin(); iterCate != tplVec.end(); ++iterCate) {
            QStandardItem *typeItem = new QStandardItem(iterCate->type);
            tpl->appendRow(typeItem);

            auto iterDetail = iterCate->templateVec.begin();
            for (; iterDetail != iterCate->templateVec.end(); ++iterDetail) {
                QStandardItem *detailItem = new QStandardItem(iterDetail->name);
                TemplateDetail detail;
                detail.name = iterDetail->name;
                detail.path = iterDetail->path;
                detail.leafNode = iterDetail->leafNode;

                detailItem->setData(QVariant::fromValue(detail), Qt::UserRole + 1);
                detailItem->setData(QVariant::fromValue(QUuid::createUuid().toString()), Qt::UserRole + 2);
                typeItem->appendRow(detailItem);
            }
        }

        treeView->setModel(standardModel);
    }

    connect(treeView, &DTreeView::clicked, [=](){
        QModelIndex index = treeView->selectionModel()->currentIndex();
        if (!index.isValid()){
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        QVariant varDetail = index.data(Qt::UserRole + 1);
        if (!varDetail.isValid()){
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        TemplateDetail detail = varDetail.value<TemplateDetail>();
        if (!detail.leafNode) {
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        QVariant varUuid = index.data(Qt::UserRole + 2);
        if (!varUuid.isValid()){
            d->detailStackedWidget->setCurrentWidget(d->blankWidget);
            return;
        }

        QString uuid = varUuid.value<QString>();
        if (d->detailWidgetMap.contains(uuid)) {
            DetailWidget * detailWidget = d->detailWidgetMap.value(uuid);
            if (detailWidget) {
                d->detailStackedWidget->setCurrentWidget(detailWidget);
            }
        } else {
            DetailWidget * detailWidget = new DetailWidget(detail.path, this);
            d->detailWidgetMap.insert(uuid, detailWidget);
            d->detailStackedWidget->addWidget(detailWidget);
            d->detailStackedWidget->setCurrentWidget(detailWidget);
        }
    });

    QVBoxLayout *vItemLayout = new QVBoxLayout();
    vItemLayout->addWidget(treeView);

    DWidget *contentWidget = new DWidget();
    QHBoxLayout *hLayout = new QHBoxLayout();
    contentWidget->setLayout(hLayout);
    hLayout->addLayout(vItemLayout);
    hLayout->addWidget(d->detailStackedWidget);
    hLayout->setStretchFactor(vItemLayout, 1);
    hLayout->setStretchFactor(d->detailStackedWidget, 2);

    DWidget *bottomWidget = new DWidget();
    QHBoxLayout * bottomLayout = new QHBoxLayout();
    bottomWidget->setLayout(bottomLayout);

    DPushButton *create = new DPushButton(tr("Create"));
    DPushButton *cancel = new DPushButton(tr("Cancel"));
    bottomLayout->addStretch(15);
    bottomLayout->addWidget(create, Qt::AlignRight);
    bottomLayout->addWidget(cancel, Qt::AlignRight);

    connect(create, &DPushButton::clicked, [&](){
        DetailWidget *detailWidget = dynamic_cast<DetailWidget*>(d->detailStackedWidget->currentWidget());
        if (detailWidget) {
            PojectGenParam param;
            if (detailWidget->getGenParams(param)) {
                generate(param);
            }
        }
    });

    connect(cancel, &DPushButton::clicked, [&](){
        close();
    });

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    vLayout->addWidget(titleBar);
    vLayout->addWidget(title);
    vLayout->addWidget(contentWidget);
    vLayout->addWidget(bottomWidget);

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
        editor.openFile(result.filePath);
    }
}
