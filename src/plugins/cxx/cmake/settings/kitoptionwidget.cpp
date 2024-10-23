// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitoptionwidget.h"
#include "global_define.h"
#include "cmake/model/kitlistmodel.h"
#include "cmake/option/kitmanager.h"

#include <DToolButton>
#include <DComboBox>
#include <DLineEdit>
#include <DListView>
#include <DFrame>
#include <DStyle>

#include <QUuid>
#include <QFormLayout>

DWIDGET_USE_NAMESPACE

class KitOptionWidgetPrivate : public QObject
{
public:
    explicit KitOptionWidgetPrivate(KitOptionWidget *qq);

    void initUI();
    void initConnection();

    QWidget *createLeftWidget();
    QWidget *createRightWidget();

    void updateItem(const QModelIndex &index);
    void showInfoForItem(const QModelIndex &index);
    void handleCurrentChanged(const QModelIndex &now, const QModelIndex &previous);
    void handleKitNameChanged(const QString &name);

    void parseKitParamsV1(const QVariantMap &config);
    void parseKitParamsV2(const QVariantMap &config);

public:
    KitOptionWidget *q;

    QWidget *rightWidget { nullptr };
    DListView *listView { nullptr };
    DLineEdit *kitNameEidt { nullptr };
    DComboBox *cbCXXComplier { nullptr };
    DComboBox *cbCMakeTool { nullptr };
    DComboBox *cbCComplier { nullptr };
    DComboBox *cbDebugger { nullptr };
    DComboBox *cbGenerator { nullptr };
    DToolButton *addKitBtn { nullptr };
    DToolButton *removeKitBtn { nullptr };
    KitListModel listModel;
};

KitOptionWidgetPrivate::KitOptionWidgetPrivate(KitOptionWidget *qq)
    : q(qq)
{
}

void KitOptionWidgetPrivate::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(createLeftWidget());
    mainLayout->addSpacing(10);
    mainLayout->addWidget(createRightWidget());
}

void KitOptionWidgetPrivate::initConnection()
{
    connect(kitNameEidt, &DLineEdit::textChanged, this, &KitOptionWidgetPrivate::handleKitNameChanged);
    connect(listView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &KitOptionWidgetPrivate::handleCurrentChanged);
    connect(addKitBtn, &DToolButton::clicked, &listModel, [this] {
        const auto &index = listModel.addItem();
        listView->setCurrentIndex(index);
    });
    connect(removeKitBtn, &DToolButton::clicked, &listModel, [this] {
        listModel.removeItem(listView->currentIndex());
    });
}

QWidget *KitOptionWidgetPrivate::createLeftWidget()
{
    DFrame *frame = new DFrame(q);
    frame->setFixedWidth(200);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    listView = new DListView(q);
    listView->setViewportMargins(5, 5, 5, 0);
    listView->setItemSpacing(1);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setModel(&listModel);

    addKitBtn = new DToolButton(q);
    addKitBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_IncreaseElement));
    addKitBtn->setIconSize({ 16, 16 });

    removeKitBtn = new DToolButton(q);
    removeKitBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_DecreaseElement));
    removeKitBtn->setIconSize({ 16, 16 });

    QHBoxLayout *btnlayout = new QHBoxLayout;
    btnlayout->setContentsMargins(8, 0, 0, 0);
    btnlayout->setSpacing(10);
    btnlayout->addWidget(addKitBtn);
    btnlayout->addWidget(removeKitBtn);
    btnlayout->setAlignment(Qt::AlignLeft);

    layout->addWidget(listView);
    layout->addWidget(new DHorizontalLine(q));
    layout->addLayout(btnlayout);

    return frame;
}

QWidget *KitOptionWidgetPrivate::createRightWidget()
{
    rightWidget = new QWidget(q);
    rightWidget->setEnabled(false);
    QFormLayout *formlayout = new QFormLayout(rightWidget);
    formlayout->setVerticalSpacing(15);
    formlayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formlayout->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    cbCXXComplier = new DComboBox(q);
    kitNameEidt = new DLineEdit(q);
    cbCMakeTool = new DComboBox(q);
    cbCComplier = new DComboBox(q);
    cbDebugger = new DComboBox(q);

    cbGenerator = new DComboBox(q);
    cbGenerator->insertItem(0, "CodeBlocks - Unix Makefiles");
    cbGenerator->setEnabled(false);

    formlayout->addRow(KitOptionWidget::tr("Name:"), kitNameEidt);
    formlayout->addRow(KitOptionWidget::tr("C Compiler:"), cbCComplier);
    formlayout->addRow(KitOptionWidget::tr("C++ Compiler:"), cbCXXComplier);
    formlayout->addRow(KitOptionWidget::tr("Debugger:"), cbDebugger);
    formlayout->addRow(KitOptionWidget::tr("CMake Tool:"), cbCMakeTool);
    formlayout->addRow(KitOptionWidget::tr("CMake generator:"), cbGenerator);

    return rightWidget;
}

void KitOptionWidgetPrivate::updateItem(const QModelIndex &index)
{
    auto item = listModel.itemForIndex(index);
    if (!item)
        return;

    auto comboBoxValue = [](DComboBox *cb) {
        int index = cb->currentIndex();
        if (index > -1) {
            return qvariant_cast<Option>(cb->itemData(index));
        } else {
            return Option();
        }
    };

    item->setKitName(kitNameEidt->text());
    item->setCCompiler(comboBoxValue(cbCComplier));
    item->setCXXCompiler(comboBoxValue(cbCXXComplier));
    item->setDebugger(comboBoxValue(cbDebugger));
    item->setCMakeTool(comboBoxValue(cbCMakeTool));
}

void KitOptionWidgetPrivate::showInfoForItem(const QModelIndex &index)
{
    auto item = listModel.itemForIndex(index);
    if (!item) {
        kitNameEidt->clear();
        cbCComplier->clear();
        cbCXXComplier->clear();
        cbDebugger->clear();
        cbCMakeTool->clear();
        rightWidget->setEnabled(false);
        return;
    }

    auto updateComplier = [](DComboBox *cb, const Option &opt) {
        for (int i = 0; i < cb->count(); ++i) {
            const auto &data = cb->itemData(i).value<Option>();
            if (data == opt) {
                cb->setCurrentIndex(i);
                return;
            }
        }

        cb->setCurrentIndex(0);
    };

    rightWidget->setEnabled(true);
    kitNameEidt->setText(item->kitName());
    updateComplier(cbCComplier, item->ccompiler());
    updateComplier(cbCXXComplier, item->cxxcompiler());
    updateComplier(cbDebugger, item->debugger());
    updateComplier(cbCMakeTool, item->cmakeTool());
}

void KitOptionWidgetPrivate::handleCurrentChanged(const QModelIndex &now, const QModelIndex &previous)
{
    updateItem(previous);
    showInfoForItem(now);
}

void KitOptionWidgetPrivate::handleKitNameChanged(const QString &name)
{
    const auto &index = listView->currentIndex();
    auto item = listModel.itemForIndex(index);
    if (!item || item->kitName() == name)
        return;

    if (!name.isEmpty()) {
        item->setKitName(name);
    } else {
        item->setKitName(KitOptionWidget::tr("Unnamed"));
        kitNameEidt->setText(item->kitName());
        kitNameEidt->lineEdit()->setCursorPosition(0);
    }

    listView->update(index);
}

void KitOptionWidgetPrivate::parseKitParamsV1(const QVariantMap &config)
{
    const auto &item = Kit::fromVariantMap(config);
    listModel.setItemList({ item });
    listView->setCurrentIndex(listModel.index(0, 0));
}

void KitOptionWidgetPrivate::parseKitParamsV2(const QVariantMap &config)
{
    QList<Kit> itemList;
    auto iter = config.begin();
    for (; iter != config.end(); ++iter) {
        auto item = Kit::fromVariantMap(iter.value().toMap());
        if (item.kitName().isEmpty())
            item.setKitName(iter.key());
        itemList.append(item);
    }

    listModel.setItemList(itemList);
    listView->setCurrentIndex(listModel.index(0, 0));
}

KitOptionWidget::KitOptionWidget(QWidget *parent)
    : QWidget(parent),
      d(new KitOptionWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

KitOptionWidget::~KitOptionWidget()
{
    delete d;
}

void KitOptionWidget::setOptions(const QPair<QString, QList<Option>> &options)
{
    if (options.first == CompilerOptionName) {
        d->cbCComplier->clear();
        d->cbCXXComplier->clear();
        d->cbCComplier->addItem(tr("None"));
        d->cbCXXComplier->addItem(tr("None"));
        for (const auto &opt : options.second) {
            const auto &item = Option::displayName(opt.name, opt.path);
            if (opt.userData.toString() == kCCompiler)
                d->cbCComplier->addItem(item, QVariant::fromValue(opt));
            else
                d->cbCXXComplier->addItem(item, QVariant::fromValue(opt));
        }

    } else if (options.first == DebuggerOptionName) {
        d->cbDebugger->clear();
        d->cbDebugger->addItem(tr("None"));
        for (const auto &opt : options.second) {
            const auto &item = Option::displayName(opt.name, opt.path);
            d->cbDebugger->addItem(item, QVariant::fromValue(opt));
        }
    } else if (options.first == CMakeToolOptionName) {
        d->cbCMakeTool->clear();
        d->cbCMakeTool->addItem(tr("None"));
        for (const auto &opt : options.second) {
            const auto &item = Option::displayName(opt.name, opt.path);
            d->cbCMakeTool->addItem(item, QVariant::fromValue(opt));
        }
    }
}

QString KitOptionWidget::titleName() const
{
    return tr("Kits");
}

QString KitOptionWidget::configName() const
{
    return KitOptionName;
}

void KitOptionWidget::setConfig(const QVariantMap &config)
{
    auto tempConfig = config;
    if (config.isEmpty()) {
        auto kit = KitManager::instance()->defaultKit();
        if (!kit.isValid())
            return;
        tempConfig.insert(kit.kitName(), kit.toVariantMap());
    }

    if (!tempConfig.first().canConvert<QVariantMap>()) {
        tempConfig.clear();
        return setConfig(tempConfig);
    }

    auto firstValue = tempConfig.first().toMap();
    if (firstValue.size() <= 2)
        d->parseKitParamsV1(tempConfig);
    else
        d->parseKitParamsV2(tempConfig);
}

QVariantMap KitOptionWidget::getConfig() const
{
    d->updateItem(d->listView->currentIndex());

    QVariantMap map;
    auto itemList = d->listModel.itemList();
    for (const auto &item : itemList) {
        const auto &params = Kit::toVariantMap(item);
        map.insert(item.kitName(), params);
    }

    KitManager::instance()->setKitList(d->listModel.itemList());
    return map;
}
