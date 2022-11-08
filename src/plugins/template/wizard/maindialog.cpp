/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "maindialog.h"

#include "detailwidget.h"

#include "services/language/languageservice.h"

#include <QLabel>
#include <QTreeView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QListWidget>
#include <QMessageBox>
#include <QDebug>
#include <QUuid>
#include <QStackedWidget>

using namespace dpfservice;

class MainDialogPrivate
{
    friend class MainDialog;
    QMap<QString, DetailWidget*> detailWidgetMap;
    QStackedWidget *detailStackedWidget = nullptr;
    QWidget *blankWidget = nullptr;
};

MainDialog::MainDialog(QDialog *parent)
    : QDialog(parent)
    , d(new MainDialogPrivate())
{
    setWindowTitle(tr("New File or Project"));
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
    QLabel * title =  new QLabel(tr("Choose a template:"));

    d->detailStackedWidget = new QStackedWidget();
    d->blankWidget = new DetailWidget(this);
    d->detailStackedWidget->addWidget(d->blankWidget);

    QTreeView * treeView =  new QTreeView();
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

    connect(treeView, &QTreeView::clicked, [=](){
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

    QWidget *contentWidget = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout();
    contentWidget->setLayout(hLayout);
    hLayout->addLayout(vItemLayout);
    hLayout->addWidget(d->detailStackedWidget);
    hLayout->setStretchFactor(vItemLayout, 1);
    hLayout->setStretchFactor(d->detailStackedWidget, 2);

    QWidget *bottomWidget = new QWidget();
    QHBoxLayout * bottomLayout = new QHBoxLayout();
    bottomWidget->setLayout(bottomLayout);

    QPushButton *create = new QPushButton(tr("Create"));
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    bottomLayout->addStretch(15);
    bottomLayout->addWidget(create, Qt::AlignRight);
    bottomLayout->addWidget(cancel, Qt::AlignRight);

    connect(create, &QPushButton::clicked, [&](){
        DetailWidget *detailWidget = dynamic_cast<DetailWidget*>(d->detailStackedWidget->currentWidget());
        if (detailWidget) {
            PojectGenParam param;
            if (detailWidget->getGenParams(param)) {
                generate(param);
            }
        }
    });

    connect(cancel, &QPushButton::clicked, [&](){
        close();
    });

    QVBoxLayout *vLayout = new QVBoxLayout();
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
        QMessageBox::critical(this, tr("Tip"), result.message);
        return;
    }

    if (param.type == Project) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            auto generator = service->create<LanguageGenerator>(result.kit);
            if (generator) {
                QString projectFile = generator->getProjectFile(result.projectPath);
                project.openProject({projectFile, result.kit, result.language, result.projectPath});

                close();
            } else {
                QMessageBox::critical(this, tr("Tip"), tr("Can not find kit."));
            }
        }
    } else if (param.type == File) {
        editor.openDocument({result.language, result.filePath});
        close();
    }
}
