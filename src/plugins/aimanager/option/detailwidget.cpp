// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "addmodeldialog.h"
#include "aimanager.h"

#include <DTableView>
#include <DComboBox>
#include <DFrame>
#include <DListView>
#include <DToolButton>
#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#include <DPaletteHelper>
#endif

#include <QAbstractTableModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QLabel>

DWIDGET_USE_NAMESPACE

LLMModels::LLMModels(QObject *parent)
    : QAbstractTableModel(parent)
{
}

LLMModels::~LLMModels()
{
}

int LLMModels::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return LLMs.size();
}

int LLMModels::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant LLMModels::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= LLMs.size())
        return QVariant();

    const LLMInfo &info = LLMs.at(index.row());

    switch (role) {
        case Qt::UserRole + 1:
            return info.modelName;
        case Qt::UserRole + 2:
            return LLMTypeToString(info.type);
        case Qt::ToolTipRole:
            return QString("Path:%1").arg(info.modelPath);
        default:
            return QVariant();
    }
}

QVariant LLMModels::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

void LLMModels::appendLLM(const LLMInfo &info)
{
    beginResetModel();
    if (!LLMs.contains(info))
        LLMs.append(info);
    endResetModel();
}

void LLMModels::removeLLM(const LLMInfo &info)
{
    beginResetModel();
    LLMs.removeOne(info);
    endResetModel();
}

QList<LLMInfo> LLMModels::allLLMs()
{
    return LLMs;
}

class DetailWidgetPrivate
{
    friend class DetailWidget;
    DListView *modelsView = nullptr;
    LLMModels *LLMModel = nullptr;
    AddModelDialog *addModelDialog = nullptr;
    DComboBox *cbCompletedLLM = nullptr;
};

DetailWidget::DetailWidget(QWidget *parent)
    : PageWidget(parent), d(new DetailWidgetPrivate())
{
    setupUi();
    addDefaultLLM();
}

DetailWidget::~DetailWidget()
{
    if (d) {
        delete d;
    }
}

void DetailWidget::setupUi()
{
    setFixedHeight(300);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *completedLLMLayout = new QHBoxLayout;
    completedLLMLayout->setContentsMargins(10, 0, 10, 0);
    d->cbCompletedLLM = new DComboBox(this);
    d->cbCompletedLLM->addItem(tr("Disabled"), QVariant());
    QLabel *lbCompletedLLM = new QLabel(tr("Auto Complete LLM:"), this);
    completedLLMLayout->addWidget(lbCompletedLLM);
    completedLLMLayout->addWidget(d->cbCompletedLLM);
    vLayout->addLayout(completedLLMLayout);

    auto listframe = new DFrame(this);
    auto listlayout = new QVBoxLayout(listframe);
    listlayout->setContentsMargins(5, 5, 5, 5);
    listframe->setLayout(listlayout);
    d->modelsView = new DListView(listframe);
    d->modelsView->setFrameShape(DFrame::NoFrame);
    d->modelsView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->modelsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->modelsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d->modelsView->setAlternatingRowColors(true);
    d->modelsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->modelsView->setItemDelegate(new TwoLineDelegate(d->modelsView));
    listlayout->addWidget(d->modelsView);

    d->LLMModel = new LLMModels(this);
    d->modelsView->setModel(d->LLMModel);

    vLayout->addWidget(listframe);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setAlignment(Qt::AlignRight);
    DToolButton *buttonAdd = new DToolButton(this);
    buttonAdd->setText(tr("Add"));
    DToolButton *buttonRemove = new DToolButton(this);
    buttonRemove->setText(tr("Remove"));
#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette pl(DPaletteHelper::instance()->palette(this));
#else
    const DPalette &pl = DGuiApplicationHelper::instance()->applicationPalette();
#endif

    QColor warningColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::TextWarning);
    QColor highlightColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::LightLively);
    auto pa = buttonRemove->palette();
    pa.setColor(QPalette::ButtonText, warningColor);
    buttonRemove->setPalette(pa);
    pa.setColor(QPalette::ButtonText, highlightColor);
    buttonAdd->setPalette(pa);

    buttonLayout->addWidget(buttonRemove);
    buttonLayout->addWidget(buttonAdd);
    vLayout->addLayout(buttonLayout);

    connect(buttonAdd, &DToolButton::clicked, this, [=](){
        auto dialog = new AddModelDialog(this);
        auto code = dialog->exec();
        if (code == QDialog::Accepted) {
            auto newLLM = dialog->getNewLLmInfo();
            d->LLMModel->appendLLM(newLLM);
            if (d->cbCompletedLLM->findData(newLLM.toVariant()) != -1)
                d->cbCompletedLLM->addItem(newLLM.modelName, newLLM.toVariant());
        }
        dialog->deleteLater();
    });
    connect(buttonRemove, &DToolButton::clicked, this, [=](){
        if (!d->modelsView->selectionModel()->hasSelection())
            return;
        QModelIndex index = d->modelsView->selectionModel()->selectedIndexes().at(0);
        if (!index.isValid())
            return;
        auto llmInfo = d->LLMModel->allLLMs().at(index.row());
        if (llmInfo.type == LLMType::ZHIPU_CODEGEEX) { // default codegeex can not remove
            DDialog dialog;
            dialog.setMessage(tr("You can't delete default CodeGeeX`s LLM!"));
            dialog.setWindowTitle(tr("Delete Warining"));
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.insertButton(0, tr("Ok"));
            dialog.exec();
            return;
        }
        d->LLMModel->removeLLM(llmInfo);
        if (d->cbCompletedLLM->findData(llmInfo.toVariant()) != -1)
            d->cbCompletedLLM->removeItem(d->cbCompletedLLM->findData(llmInfo.toVariant()));
    });
}

void DetailWidget::addDefaultLLM()
{
    auto LLMs = AiManager::instance()->getDefaultLLM();
    for (auto llm : LLMs) {
        d->LLMModel->appendLLM(llm);
        d->cbCompletedLLM->addItem(llm.modelName, llm.toVariant());
    }
}

bool DetailWidget::getControlValue(QMap<QString, QVariant> &map)
{
    QVariantList LLMs;
    for (auto llmInfo : d->LLMModel->allLLMs()) {
        LLMs.append(llmInfo.toVariant());
    }

    map.insert(kCATEGORY_CUSTOMMODELS, LLMs);
    map.insert(kCATEGORY_AUTO_COMPLETE, d->cbCompletedLLM->currentData());
    return true;
}

void DetailWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    for (auto mapData : map.value(kCATEGORY_CUSTOMMODELS).toList()) {
        auto llmInfo = LLMInfo::fromVariantMap(mapData.toMap());
        if (llmInfo.type == LLMType::ZHIPU_CODEGEEX) // default model is already exist
            continue;
        d->LLMModel->appendLLM(llmInfo);
        if (d->cbCompletedLLM->findData(llmInfo.toVariant()) == -1)
            d->cbCompletedLLM->addItem(llmInfo.modelName, llmInfo.toVariant());
    }
    if (!map.value(kCATEGORY_AUTO_COMPLETE).isValid()) {
        d->cbCompletedLLM->setCurrentIndex(0);
    } else {
        auto selectedCompleteLLM = LLMInfo::fromVariantMap(map.value(kCATEGORY_AUTO_COMPLETE).toMap());
        d->cbCompletedLLM->setCurrentText(selectedCompleteLLM.modelName);
    }
}

void DetailWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void DetailWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}
