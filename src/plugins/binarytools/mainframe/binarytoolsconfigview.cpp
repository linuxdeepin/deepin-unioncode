// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytoolsconfigview.h"
#include "environmentview.h"
#include "models/binarytoolsmodel.h"
#include "configure/binarytoolsmanager.h"
#include "widgets/combinationcomboxbox.h"
#include "widgets/iconcombobox.h"
#include "constants.h"

#include <DComboBox>
#include <DCheckBox>
#include <DLineEdit>
#include <DToolButton>
#include <DFileDialog>
#include <DLabel>
#include <DSuggestButton>
#include <DTreeView>
#include <DFrame>

#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QListView>
#include <QStandardPaths>

DWIDGET_USE_NAMESPACE

class BinaryToolsConfigViewPrivate : public QObject
{
public:
    enum ItemRow {
        DescriptionRow = 0,
        CombinationRow,
        CommandRow,
        ArgumentsRow,
        WorkingDirRow,
        OutputRow,
        ErrorOutputRow,
        AddToToolbarRow,
        IconRow,
        EnvironmentLabelRow,
        EnvironmentRow
    };

    explicit BinaryToolsConfigViewPrivate(BinaryToolsConfigView *qq);

    void initUI();
    void initConnections();
    void initModel();

    QWidget *createLeftWidget();
    QWidget *createRightWidget();

    void updateItem(const QModelIndex &index);
    void showInfoForItem(const QModelIndex &index);
    void updateItemVisible(int type);

    void handleCurrentChanged(const QModelIndex &now, const QModelIndex &previous);
    void handleAddTool();
    void handleAddCombination();
    void handleAddGroup();
    void handleRemove();
    void handleSelectCommand();
    void handleSelectWorkingDirectory();

public:
    BinaryToolsConfigView *q;
    QModelIndex currentIndex;

    // left
    DTreeView *toolTree = nullptr;
    BinaryToolsModel treeModel;
    DToolButton *addBtn = nullptr;
    DToolButton *delBtn = nullptr;

    // right
    QWidget *rightWidget = nullptr;
    DLineEdit *descriptionEdit = nullptr;
    CombinationComboxBox *combinationCB = nullptr;
    DLineEdit *commandEdit = nullptr;
    DSuggestButton *selCmdBtn = nullptr;
    DLineEdit *argsEdit = nullptr;
    DLineEdit *workingDirEdit = nullptr;
    DSuggestButton *selWorkDirBtn = nullptr;
    DComboBox *outputCB = nullptr;
    DComboBox *errOutputCB = nullptr;
    DCheckBox *addToolbarCB = nullptr;
    IconComboBox *iconCB = nullptr;
    EnvironmentView *envView = nullptr;
};

BinaryToolsConfigViewPrivate::BinaryToolsConfigViewPrivate(BinaryToolsConfigView *qq)
    : q(qq)
{
}

void BinaryToolsConfigViewPrivate::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(30);

    auto leftWidget = createLeftWidget();
    rightWidget = createRightWidget();
    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(rightWidget, 1);

    rightWidget->setEnabled(false);
    updateItemVisible(CommandTool);
}

void BinaryToolsConfigViewPrivate::initConnections()
{
    connect(toolTree->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &BinaryToolsConfigViewPrivate::handleCurrentChanged);
    connect(delBtn, &DToolButton::clicked, this, &BinaryToolsConfigViewPrivate::handleRemove);
    connect(selCmdBtn, &DSuggestButton::clicked, this, &BinaryToolsConfigViewPrivate::handleSelectCommand);
    connect(selWorkDirBtn, &DSuggestButton::clicked, this, &BinaryToolsConfigViewPrivate::handleSelectWorkingDirectory);
}

void BinaryToolsConfigViewPrivate::initModel()
{
    const auto &tools = BinaryToolsManager::instance()->tools();
    treeModel.setTools(tools);
    toolTree->expandAll();

    combinationCB->setTools(tools);
}

QWidget *BinaryToolsConfigViewPrivate::createLeftWidget()
{
    DFrame *widget = new DFrame(q);
    widget->setFixedWidth(200);
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(10, 10, 10, 6);
    mainLayout->setSpacing(0);

    toolTree = new DTreeView(q);
    toolTree->setFrameShape(QFrame::NoFrame);
    toolTree->setDragEnabled(false);
    toolTree->header()->setVisible(false);
    toolTree->setModel(&treeModel);
    toolTree->setIconSize({ 16, 16 });
    toolTree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    addBtn = new DToolButton(q);
    addBtn->setIconSize({ 16, 16 });
    addBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_IncreaseElement));
    addBtn->setPopupMode(QToolButton::InstantPopup);
    QMenu *addMenu = new QMenu(q);
    addMenu->addAction(BinaryToolsConfigView::tr("Add Tool"), this, &BinaryToolsConfigViewPrivate::handleAddTool);
    // TODO: add combination
    // addMenu->addAction(BinaryToolsConfigView::tr("Add Combination"), this, &BinaryToolsConfigViewPrivate::handleAddCombination);
    addMenu->addSeparator();
    addMenu->addAction(BinaryToolsConfigView::tr("Add Group"), this, &BinaryToolsConfigViewPrivate::handleAddGroup);
    addBtn->setMenu(addMenu);

    delBtn = new DToolButton(q);
    delBtn->setIconSize({ 16, 16 });
    delBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_DecreaseElement));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addStretch(1);

    mainLayout->addWidget(toolTree);
    mainLayout->addLayout(btnLayout);
    return widget;
}

QWidget *BinaryToolsConfigViewPrivate::createRightWidget()
{
    QWidget *widget = new QWidget(q);
    QGridLayout *mainLayout = new QGridLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    DLabel *descriptionLabel = new DLabel(BinaryToolsConfigView::tr("Description:"), q);
    descriptionEdit = new DLineEdit(q);
    mainLayout->addWidget(descriptionLabel, DescriptionRow, 0);
    mainLayout->addWidget(descriptionEdit, DescriptionRow, 1, 1, 2);

    DLabel *combinationLabel = new DLabel(BinaryToolsConfigView::tr("Combination:"), q);
    combinationCB = new CombinationComboxBox(q);
    mainLayout->addWidget(combinationLabel, CombinationRow, 0);
    mainLayout->addWidget(combinationCB, CombinationRow, 1, 1, 2);

    DLabel *commandLabel = new DLabel(BinaryToolsConfigView::tr("Executable:"), q);
    commandEdit = new DLineEdit(q);
    selCmdBtn = new DSuggestButton(q);
    selCmdBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_SelectElement));
    mainLayout->addWidget(commandLabel, CommandRow, 0);
    mainLayout->addWidget(commandEdit, CommandRow, 1);
    mainLayout->addWidget(selCmdBtn, CommandRow, 2);

    DLabel *argsLabel = new DLabel(BinaryToolsConfigView::tr("Arguments:"), q);
    argsEdit = new DLineEdit(q);
    mainLayout->addWidget(argsLabel, ArgumentsRow, 0);
    mainLayout->addWidget(argsEdit, ArgumentsRow, 1, 1, 2);

    DLabel *workDirLabel = new DLabel(BinaryToolsConfigView::tr("Working directory:"), q);
    workingDirEdit = new DLineEdit(q);
    selWorkDirBtn = new DSuggestButton(q);
    selWorkDirBtn->setIcon(DStyle::standardIcon(q->style(), DStyle::SP_SelectElement));
    mainLayout->addWidget(workDirLabel, WorkingDirRow, 0);
    mainLayout->addWidget(workingDirEdit, WorkingDirRow, 1);
    mainLayout->addWidget(selWorkDirBtn, WorkingDirRow, 2);

    DLabel *outputLabel = new DLabel(BinaryToolsConfigView::tr("Output:"), q);
    outputCB = new DComboBox(q);
    outputCB->setFixedWidth(240);
    outputCB->addItem(BinaryToolsConfigView::tr("Ignore"), Ignore);
    outputCB->addItem(BinaryToolsConfigView::tr("Show in application output"), ShowInApplicationOutput);
    mainLayout->addWidget(outputLabel, OutputRow, 0);
    mainLayout->addWidget(outputCB, OutputRow, 1);

    DLabel *errOutputLabel = new DLabel(BinaryToolsConfigView::tr("Error output:"), q);
    errOutputCB = new DComboBox(q);
    errOutputCB->setFixedWidth(240);
    errOutputCB->addItem(BinaryToolsConfigView::tr("Ignore"), Ignore);
    errOutputCB->addItem(BinaryToolsConfigView::tr("Show in application output"), ShowInApplicationOutput);
    mainLayout->addWidget(errOutputLabel, ErrorOutputRow, 0);
    mainLayout->addWidget(errOutputCB, ErrorOutputRow, 1);

    addToolbarCB = new DCheckBox(BinaryToolsConfigView::tr("Add this command to the toolbar"), q);
    mainLayout->addWidget(addToolbarCB, AddToToolbarRow, 0, 1, 2);

    DLabel *cmdIconLabel = new DLabel(BinaryToolsConfigView::tr("Tool icon:"), q);
    iconCB = new IconComboBox(q);
    iconCB->setFixedWidth(240);
    mainLayout->addWidget(cmdIconLabel, IconRow, 0);
    mainLayout->addWidget(iconCB, IconRow, 1);

    DLabel *envLabel = new DLabel(BinaryToolsConfigView::tr("Configuration environment for the current command:"), q);
    envView = new EnvironmentView(q);
    auto *envViewFrame = new DFrame(q);
    QVBoxLayout *envViewlayout = new QVBoxLayout(envViewFrame);
    envViewlayout->setContentsMargins(5, 5, 5, 5);
    envViewFrame->setLayout(envViewlayout);
    envViewlayout->addWidget(envView);
    mainLayout->addWidget(envLabel, EnvironmentLabelRow, 0, 1, 3);
    mainLayout->addWidget(envViewFrame, EnvironmentRow, 0, 1, 3);
    mainLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Maximum, QSizePolicy::MinimumExpanding), EnvironmentRow + 1, 0);

    return widget;
}

void BinaryToolsConfigViewPrivate::updateItem(const QModelIndex &index)
{
    auto tool = treeModel.toolForIndex(index);
    if (!tool)
        return;

    tool->description = descriptionEdit->text();
    tool->outputOption = outputCB->currentData().toInt();
    tool->errorOutputOption = errOutputCB->currentData().toInt();
    tool->addToToolbar = addToolbarCB->isChecked();
    tool->icon = iconCB->icon();

    switch (tool->type) {
    case CommandTool:
        tool->command = commandEdit->text();
        tool->arguments = argsEdit->text();
        tool->workingDirectory = workingDirEdit->text();
        tool->environment = envView->getEnvironment();
        break;
    case CombinationTool:
        tool->command = combinationCB->text();
        break;
    }
}

void BinaryToolsConfigViewPrivate::showInfoForItem(const QModelIndex &index)
{
    currentIndex = index;
    const auto tool = treeModel.toolForIndex(index);
    if (!tool) {
        descriptionEdit->clear();
        commandEdit->clear();
        argsEdit->clear();
        workingDirEdit->clear();
        combinationCB->clear();
        rightWidget->setEnabled(false);
        envView->setValue({});
        return;
    }

    rightWidget->setEnabled(true);
    updateItemVisible(tool->type);

    descriptionEdit->setText(tool->description);
    outputCB->setCurrentIndex(outputCB->findData(tool->outputOption));
    errOutputCB->setCurrentIndex(outputCB->findData(tool->errorOutputOption));
    addToolbarCB->setChecked(tool->addToToolbar);
    iconCB->setIcon(tool->icon);

    switch (tool->type) {
    case CommandTool:
        commandEdit->setText(tool->command);
        argsEdit->setText(tool->arguments);
        workingDirEdit->setText(tool->workingDirectory);
        envView->setValue(tool->environment);
        break;
    case CombinationTool:
        combinationCB->setText(tool->command);
    default:
        break;
    }
}

void BinaryToolsConfigViewPrivate::updateItemVisible(int type)
{
    auto layout = qobject_cast<QGridLayout *>(rightWidget->layout());
    if (!layout)
        return;

    auto setVisible = [layout](ItemRow row, bool visible) {
        int count = layout->columnCount();
        for (int i = 0; i < count; ++i) {
            auto item = layout->itemAtPosition(row, i);
            if (item)
                item->widget()->setVisible(visible);
        }
    };

    bool visible = type == CommandTool;

    setVisible(CommandRow, visible);
    setVisible(ArgumentsRow, visible);
    setVisible(WorkingDirRow, visible);
    setVisible(EnvironmentLabelRow, visible);
    setVisible(EnvironmentRow, visible);
    setVisible(CombinationRow, !visible);
}

void BinaryToolsConfigViewPrivate::handleCurrentChanged(const QModelIndex &now, const QModelIndex &previous)
{
    updateItem(previous);
    showInfoForItem(now);
}

void BinaryToolsConfigViewPrivate::handleAddTool()
{
    QModelIndex currentIndex = toolTree->selectionModel()->currentIndex();
    if (!currentIndex.isValid())
        currentIndex = treeModel.index(0, 0);

    QModelIndex index = treeModel.addTool(currentIndex);
    toolTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear);
    toolTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    toolTree->edit(index);
}

void BinaryToolsConfigViewPrivate::handleAddCombination()
{
    QModelIndex currentIndex = toolTree->selectionModel()->currentIndex();
    if (!currentIndex.isValid())
        currentIndex = treeModel.index(0, 0);

    QModelIndex index = treeModel.addCombination(currentIndex);
    toolTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear);
    toolTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    toolTree->edit(index);
}

void BinaryToolsConfigViewPrivate::handleAddGroup()
{
    QModelIndex index = treeModel.addGroup();
    toolTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear);
    toolTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    toolTree->edit(index);
}

void BinaryToolsConfigViewPrivate::handleRemove()
{
    QModelIndex currentIndex = toolTree->selectionModel()->currentIndex();
    toolTree->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);

    if (currentIndex.parent().isValid())
        treeModel.removeTool(currentIndex);
    else
        treeModel.removeGroup(currentIndex);
}

void BinaryToolsConfigViewPrivate::handleSelectCommand()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getOpenFileName(q, tr("Select Executabel Path"), dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;

    commandEdit->setText(filePath);
}

void BinaryToolsConfigViewPrivate::handleSelectWorkingDirectory()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getExistingDirectory(q, tr("Select Working Directory"), dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;

    workingDirEdit->setText(filePath);
}

BinaryToolsConfigView::BinaryToolsConfigView(QWidget *parent)
    : QWidget(parent),
      d(new BinaryToolsConfigViewPrivate(this))
{
    d->initUI();
    d->initConnections();
    d->initModel();
}

BinaryToolsConfigView::~BinaryToolsConfigView()
{
    if (d)
        delete d;
}

bool BinaryToolsConfigView::saveConfig()
{
    d->updateItem(d->currentIndex);
    const auto &tools = d->treeModel.tools();

    BinaryToolsManager::instance()->setTools(tools);
    BinaryToolsManager::instance()->checkAndAddToToolbar(tools);
    BinaryToolsManager::instance()->updateToolMenu(tools);
    BinaryToolsManager::instance()->save();
    return true;
}
