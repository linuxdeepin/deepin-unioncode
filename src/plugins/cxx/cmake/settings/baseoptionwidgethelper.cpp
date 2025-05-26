// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseoptionwidgethelper.h"

#include "base/baseitemdelegate.h"

#include <DFrame>
#include <DLabel>
#include <DToolButton>
#include <DLineEdit>
#include <DSuggestButton>

#include <QFileDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QStandardPaths>

DWIDGET_USE_NAMESPACE

class BaseOptionWidgetHelperPrivate : public QObject
{
public:
    explicit BaseOptionWidgetHelperPrivate(BaseOptionWidgetHelper *qq);

    void initUI();
    void initConnection();

    QWidget *createOptionInfoWidget();
    void updateItem(const QModelIndex &index);
    void showInfoForItem(const QModelIndex &index);
    void handleCurrentChanged(const QModelIndex &now, const QModelIndex &previous);
    void handleRemoveItem();

public:
    BaseOptionWidgetHelper *q;

    QStandardItemModel optionModel;
    QTreeView *optionView { nullptr };
    DToolButton *addOptionBtn { nullptr };
    DToolButton *delOptionBtn { nullptr };

    DLineEdit *optionNameEdit { nullptr };
    DLineEdit *optionBrowseEdit { nullptr };
    DSuggestButton *browseBtn { nullptr };
};

BaseOptionWidgetHelperPrivate::BaseOptionWidgetHelperPrivate(BaseOptionWidgetHelper *qq)
    : q(qq)
{
}

void BaseOptionWidgetHelperPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    optionView = new QTreeView(q);
    optionView->setAlternatingRowColors(true);
    optionView->setEditTriggers(QTreeView::NoEditTriggers);
    optionView->setFrameShape(QFrame::NoFrame);
    optionView->setHeaderHidden(true);
    optionView->setModel(&optionModel);
    optionView->setItemDelegate(new BaseItemDelegate(optionView));

    addOptionBtn = new DToolButton(q);
    addOptionBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_IncreaseElement));
    addOptionBtn->setIconSize({ 16, 16 });

    delOptionBtn = new DToolButton(q);
    delOptionBtn->setIconSize({ 16, 16 });
    delOptionBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::DStyle::SP_DecreaseElement));
    delOptionBtn->setEnabled(false);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(10, 0, 10, 0);
    btnLayout->addWidget(addOptionBtn);
    btnLayout->addWidget(delOptionBtn);
    btnLayout->addStretch(1);

    DFrame *viewFrame = new DFrame(q);
    QVBoxLayout *viewLayout = new QVBoxLayout(viewFrame);
    viewLayout->setContentsMargins(0, 8, 0, 0);
    viewLayout->setSpacing(0);
    viewLayout->addWidget(optionView);
    viewLayout->addWidget(new DHorizontalLine(q));
    viewLayout->addLayout(btnLayout);

    mainLayout->addWidget(viewFrame);
    mainLayout->addWidget(createOptionInfoWidget());
}

void BaseOptionWidgetHelperPrivate::initConnection()
{
    connect(optionView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &BaseOptionWidgetHelperPrivate::handleCurrentChanged);
    connect(browseBtn, &DSuggestButton::clicked, this, [this] {
        const auto &fileName = QFileDialog::getOpenFileName(q, BaseOptionWidgetHelper::tr("Choose Executable"),
                                                            QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        if (!fileName.isEmpty())
            optionBrowseEdit->setText(fileName);
    });
    connect(addOptionBtn, &DToolButton::clicked, q, &BaseOptionWidgetHelper::requestAddOption);
    connect(delOptionBtn, &DToolButton::clicked, this, &BaseOptionWidgetHelperPrivate::handleRemoveItem);
}

QWidget *BaseOptionWidgetHelperPrivate::createOptionInfoWidget()
{
    QWidget *widget = new QWidget(q);
    QFormLayout *formLayout = new QFormLayout(widget);

    optionNameEdit = new DLineEdit(q);
    optionBrowseEdit = new DLineEdit(q);
    browseBtn = new DSuggestButton(q);
    browseBtn->setIconSize({ 16, 16 });
    browseBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_SelectElement));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(optionBrowseEdit, 1);
    layout->addWidget(browseBtn);

    formLayout->addRow(BaseOptionWidgetHelper::tr("Name:"), optionNameEdit);
    formLayout->addRow(BaseOptionWidgetHelper::tr("Path:"), layout);

    return widget;
}

void BaseOptionWidgetHelperPrivate::updateItem(const QModelIndex &index)
{
    auto item = optionModel.itemFromIndex(index);
    if (!item || item->hasChildren() || !item->data(OptionTypeRole).isValid())
        return;

    if (item->data(OptionTypeRole).toInt() == Detected)
        return;

    const auto &name = optionNameEdit->text();
    const auto &path = optionBrowseEdit->text();

    QString displayText;
    if (path.isEmpty())
        displayText = name;
    else
        displayText = QString("%1 (%2)").arg(name, path);

    item->setText(displayText);
    item->setData(name, OptionNameRole);
    item->setData(path, OptionPathRole);
    q->checkAndSetItemValid(item);
}

void BaseOptionWidgetHelperPrivate::showInfoForItem(const QModelIndex &index)
{
    auto item = optionModel.itemFromIndex(index);
    if (!item || item->hasChildren() || !item->data(OptionTypeRole).isValid()) {
        q->setState(false, true);
        delOptionBtn->setEnabled(false);
        return;
    }

    bool isManualItem = item->data(OptionTypeRole).toInt() == Manual;
    q->setState(isManualItem, false);
    delOptionBtn->setEnabled(isManualItem);
    q->checkAndSetItemValid(item);

    optionNameEdit->setText(item->data(OptionNameRole).toString());
    optionBrowseEdit->setText(item->data(OptionPathRole).toString());
}

void BaseOptionWidgetHelperPrivate::handleCurrentChanged(const QModelIndex &now, const QModelIndex &previous)
{
    updateItem(previous);
    showInfoForItem(now);
}

void BaseOptionWidgetHelperPrivate::handleRemoveItem()
{
    auto index = optionView->currentIndex();
    auto item = optionModel.itemFromIndex(index);
    if (!item || !item->data(OptionTypeRole).isValid()
        || item->data(OptionTypeRole).toInt() == Detected)
        return;

    optionModel.removeRow(index.row(), index.parent());
}

BaseOptionWidgetHelper::BaseOptionWidgetHelper(QWidget *parent)
    : QWidget(parent),
      d(new BaseOptionWidgetHelperPrivate(this))
{
    d->initUI();
    d->initConnection();
}

BaseOptionWidgetHelper::~BaseOptionWidgetHelper()
{
    delete d;
}

void BaseOptionWidgetHelper::addItem(QStandardItem *item)
{
    d->optionModel.appendRow(item);
}

void BaseOptionWidgetHelper::setAddButtonMenu(QMenu *menu)
{
    d->addOptionBtn->setMenu(menu);
    d->addOptionBtn->setPopupMode(QToolButton::InstantPopup);
}

void BaseOptionWidgetHelper::setState(bool enabled, bool clear)
{
    d->optionNameEdit->setEnabled(enabled);
    d->optionBrowseEdit->setEnabled(enabled);
    d->browseBtn->setEnabled(enabled);

    if (clear) {
        d->optionNameEdit->clear();
        d->optionBrowseEdit->clear();
    }
}

void BaseOptionWidgetHelper::checkAndSetItemValid(QStandardItem *item)
{
    const auto &path = item->data(OptionPathRole).toString();
    if (path.isEmpty()) {
        item->setData(false, OptionValidRole);
        item->setIcon(QIcon::fromTheme("error"));
        item->setToolTip(BaseOptionWidgetHelper::tr("This toolchain is invalid"));
    } else if (!item->data(OptionValidRole).toBool()) {
        item->setData(true, OptionValidRole);
        item->setIcon({});
        item->setToolTip("");
    }
}

void BaseOptionWidgetHelper::applyCurrentOption() const
{
    auto index = d->optionView->currentIndex();
    d->updateItem(index);
}

void BaseOptionWidgetHelper::clearItem(QStandardItem *item)
{
    while (item->hasChildren()) {
        item->removeRow(0);
    }
}

void BaseOptionWidgetHelper::expandAll()
{
    d->optionView->expandAll();
}

void BaseOptionWidgetHelper::sort(Qt::SortOrder order)
{
    d->optionView->sortByColumn(0, order);
}

QModelIndex BaseOptionWidgetHelper::indexFromItem(const QStandardItem *item)
{
    return d->optionModel.indexFromItem(item);
}

void BaseOptionWidgetHelper::setCurrentIndex(const QModelIndex &index)
{
    d->optionView->setCurrentIndex(index);
}

void BaseOptionWidgetHelper::hideEvent(QHideEvent *event)
{
    applyCurrentOption();
    QWidget::hideEvent(event);
}
