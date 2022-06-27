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

#include "shortcutsettingwidget.h"
#include "common/common.h"

#include <QtWidgets/QTableView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QDebug>
#include <QDir>
#include <QFileDialog>

#define BTN_WIDTH (180)

class ShortcutTableModelPrivate
{
    QMap<QString, QStringList> shortcutItemMap;
    QMap<QString, QStringList> shortcutItemShadowMap;
    QString configFilePath;

    friend class ShortcutTableModel;
};

ShortcutTableModel::ShortcutTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d(new ShortcutTableModelPrivate())
{
    d->configFilePath = (CustomPaths::user(CustomPaths::Flags::Configures)
                         + QDir::separator() + QString("shortcut.support"));
}

ShortcutTableModel::~ShortcutTableModel()
{

}

int ShortcutTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return d->shortcutItemMap.size();
}

int ShortcutTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return ColumnID::_KCount;
}


bool ShortcutTableModel::keySequenceIsInvalid(const QKeySequence &sequence) const
{
    for (uint i = 0; i < static_cast<uint>(sequence.count()); i++) {
        if (Qt::Key_unknown == sequence[i]) {
            return true;
        }
    }

    return false;
}

bool ShortcutTableModel::shortcutRepeat(const QString &text) const
{
    int count  = 0;
    foreach (QString key, d->shortcutItemMap.keys()) {
        QStringList valueList = d->shortcutItemMap.value(key);
        if (0 == text.compare(valueList.last(), Qt::CaseInsensitive)) {
            if (count++ > 0)
                return true;
        }
    }

    return false;
}

QVariant ShortcutTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::TextColorRole)
        return QVariant();

    if (index.row() >= d->shortcutItemMap.keys().size())
        return QVariant();

    QString id = d->shortcutItemMap.keys()[index.row()];
    QStringList valueList = d->shortcutItemMap.value(id);

    QString description = valueList.first();
    QString shortcut = valueList.last();

    if(role == Qt::TextColorRole && index.column() == ColumnID::kShortcut) {
        if(shortcutRepeat(shortcut) || keySequenceIsInvalid(QKeySequence(shortcut))) {
            return QColor(Qt::darkRed);
        }
        return QVariant();
    }

    switch (index.column()) {
        case ColumnID::kID:
            return id;
        case ColumnID::kDescriptions:
            return description;
        case ColumnID::kShortcut:
            return shortcut;
        default:
            return QVariant();
    }
}

QVariant ShortcutTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case ColumnID::kID:
            return tr("ID");
        case ColumnID::kDescriptions:
            return tr("Description");
        case ColumnID::kShortcut:
            return tr("Shortcut");
        default:
            return QVariant();
    }
}

void ShortcutTableModel::updateShortcut(QString id, QString shortcut)
{
    if (d->shortcutItemMap.keys().contains(id))
    {
        QStringList valueList = d->shortcutItemMap.value(id);
        QStringList newValueList = {valueList.first(), shortcut};
        d->shortcutItemMap[id] = newValueList;
    }
}

void ShortcutTableModel::resetShortcut(QString id)
{
    if (d->shortcutItemMap.keys().contains(id) && d->shortcutItemShadowMap.keys().contains(id))
    {
        QStringList shadowValueList = d->shortcutItemShadowMap.value(id);
        d->shortcutItemMap[id] = shadowValueList;
    }
}

void ShortcutTableModel::resetAllShortcut()
{
    d->shortcutItemMap = d->shortcutItemShadowMap;
}

void ShortcutTableModel::saveShortcut()
{
    ShortcutUtil::writeToJson(d->configFilePath, d->shortcutItemMap);

    QList<Command *> commandsList = ActionManager::getInstance()->commands();
    QList<Command *>::iterator iter = commandsList.begin();
    for (; iter != commandsList.end(); ++iter)
    {
        Action * action = dynamic_cast<Action *>(*iter);
        QString id = action->id();

        if (d->shortcutItemMap.contains(id)) {
            QStringList valueList = d->shortcutItemMap[id];
            action->setKeySequence(QKeySequence(valueList.last()));
        }
    }
}

void ShortcutTableModel::readShortcut()
{
    beginResetModel();

    QList<Command *> commandsList = ActionManager::getInstance()->commands();
    QList<Command *>::iterator iter = commandsList.begin();
    for (; iter != commandsList.end(); ++iter)
    {
        Action * action = dynamic_cast<Action *>(*iter);
        QString id = action->id();
        QStringList valueList = QStringList{action->description(), action->keySequence().toString()};
        d->shortcutItemMap[id] = valueList;
    }

    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(d->configFilePath, shortcutItemMap);
    foreach (const QString key, shortcutItemMap.keys()) {
        d->shortcutItemMap[key] = shortcutItemMap.value(key);
    }

    d->shortcutItemShadowMap = d->shortcutItemMap;

    endResetModel();
}

void ShortcutTableModel::importExternalJson(const QString &filePath)
{
    beginResetModel();

    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(filePath, shortcutItemMap);
    foreach (QString key, shortcutItemMap.keys()) {
        d->shortcutItemMap[key] = shortcutItemMap.value(key);
    }

    d->shortcutItemShadowMap = d->shortcutItemMap;

    endResetModel();
}

void ShortcutTableModel::exportExternalJson(const QString &filePath)
{
    ShortcutUtil::writeToJson(filePath, d->shortcutItemMap);
}

class ShortcutTableModel;
class ShortcutSettingWidgetPrivate
{
    ShortcutSettingWidgetPrivate();
    QTableView *tableView;
    HotkeyLineEdit *editShortCut;
    ShortcutTableModel *model;
    QPushButton *btnRecord;
    QLabel *tipLabel;

    friend class ShortcutSettingWidget;
};

ShortcutSettingWidgetPrivate::ShortcutSettingWidgetPrivate()
    : tableView(nullptr)
    , editShortCut(nullptr)
    , model(nullptr)
    , btnRecord(nullptr)
    , tipLabel(nullptr)
{

}

ShortcutSettingWidget::ShortcutSettingWidget(QWidget *parent)
    : QWidget(parent)
    , d(new ShortcutSettingWidgetPrivate())
{
    setupUi();
    readConfig();
}

ShortcutSettingWidget::~ShortcutSettingWidget()
{

}

void ShortcutSettingWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    d->tableView = new QTableView();
    d->tableView->setShowGrid(false);
    d->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    d->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->model = new ShortcutTableModel();
    d->tableView->setModel(d->model);
    vLayout->addWidget(d->tableView);

    QWidget *widgetOperate = new QWidget();
    vLayout->addWidget(widgetOperate);
    QHBoxLayout *hLayoutOperate = new QHBoxLayout();
    widgetOperate->setLayout(hLayoutOperate);
    QPushButton *btnResetAll = new QPushButton();
    btnResetAll->setText("Reset All");
    btnResetAll->setFixedWidth(BTN_WIDTH);
    QPushButton *btnImport = new QPushButton();
    btnImport->setText("Import");
    btnImport->setFixedWidth(BTN_WIDTH);
    QPushButton *btnExport = new QPushButton();
    btnExport->setText("Export");
    btnExport->setFixedWidth(BTN_WIDTH);
    hLayoutOperate->addWidget(btnResetAll);
    hLayoutOperate->addStretch();
    hLayoutOperate->addWidget(btnImport);
    hLayoutOperate->addWidget(btnExport);

    QWidget *widgetShortcut = new QWidget();
    vLayout->addWidget(widgetShortcut);
    QHBoxLayout *hLayoutShortcut = new QHBoxLayout();
    widgetShortcut->setLayout(hLayoutShortcut);
    QLabel *labelTip = new QLabel("Shortcut:");
    d->editShortCut = new HotkeyLineEdit();
    d->btnRecord = new QPushButton("Record");
    d->btnRecord->setFixedWidth(BTN_WIDTH);
    QPushButton *btnReset = new QPushButton("Reset");
    btnReset->setFixedWidth(BTN_WIDTH);
    hLayoutShortcut->addWidget(labelTip);
    hLayoutShortcut->addWidget(d->editShortCut);
    hLayoutShortcut->addWidget(d->btnRecord);
    hLayoutShortcut->addWidget(btnReset);

    d->tipLabel = new QLabel();
    d->tipLabel->setMargin(10);
    d->tipLabel->setStyleSheet("color:darkred;");
    vLayout->addWidget(d->tipLabel);

    connect(d->tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onTableViewClicked(const QModelIndex &)));
    connect(btnResetAll, SIGNAL(clicked()), this, SLOT(onBtnResetAllClicked()));
    connect(d->editShortCut, SIGNAL(textChanged(const QString &)), this, SLOT(onShortcutEditTextChanged(const QString &)));
    connect(btnImport, SIGNAL(clicked()), this, SLOT(onBtnImportClicked()));
    connect(btnExport, SIGNAL(clicked()), this, SLOT(onBtnExportClicked()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(onBtnResetClicked()));
    connect(d->btnRecord, SIGNAL(clicked()), this, SLOT(onBtnRecordClicked()));
}

void ShortcutSettingWidget::setSelectedShortcut()
{
    int row = d->tableView->currentIndex().row();
    QModelIndex index = d->model->index(row, ColumnID::kShortcut);
    QString shortcut = d->model->data(index, Qt::DisplayRole).toString();
    d->editShortCut->setText(shortcut);

    checkShortcutValidity(row, shortcut);
}

bool ShortcutSettingWidget::shortcutIsRepeat(const int row, const QString &text)
{
    for (int i = 0; i < d->model->rowCount(); i++) {
        if (row == i)
            continue;
        QModelIndex index = d->model->index(i, ColumnID::kShortcut);
        QString shortcut = d->model->data(index, Qt::DisplayRole).toString();
        if (text == shortcut) {
            return true;
        }
    }

    return false;
}

void ShortcutSettingWidget::checkShortcutValidity(const int row, const QString &shortcut)
{
    if (d->model->keySequenceIsInvalid(QKeySequence(shortcut))) {
        d->tipLabel->setText("Invalid shortcut!");
    } else if (shortcutIsRepeat(row, shortcut)){
        d->tipLabel->setText("shortcut Repeated!");
    } else {
        d->tipLabel->setText("");
    }
}

void ShortcutSettingWidget::onTableViewClicked(const QModelIndex &)
{
    setSelectedShortcut();
}

void ShortcutSettingWidget::onBtnResetAllClicked()
{
    d->model->resetAllShortcut();
    d->tableView->update();
}

void ShortcutSettingWidget::onBtnResetClicked()
{
    int row = d->tableView->currentIndex().row();
    QModelIndex indexID = d->model->index(row, ColumnID::kID);
    QString qsID = d->model->data(indexID, Qt::DisplayRole).toString();
    d->model->resetShortcut(qsID);

    setSelectedShortcut();
}

void ShortcutSettingWidget::onShortcutEditTextChanged(const QString &text)
{
    QString shortcut = text.trimmed();
    int row = d->tableView->currentIndex().row();
    QModelIndex indexID = d->model->index(row, ColumnID::kID);
    QString qsID = d->model->data(indexID, Qt::DisplayRole).toString();
    d->model->updateShortcut(qsID, shortcut);

    QModelIndex indexShortcut = d->model->index(row, ColumnID::kShortcut);
    d->tableView->update(indexShortcut);

    checkShortcutValidity(row, shortcut);
}

void ShortcutSettingWidget::saveConfig()
{
    d->model->saveShortcut();
}

void ShortcutSettingWidget::readConfig()
{
    d->model->readShortcut();
}

void ShortcutSettingWidget::onBtnImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), tr(""), tr("Json File(*.json)"));
    if (!fileName.isEmpty()) {
        d->model->importExternalJson(fileName);
    }
}

void ShortcutSettingWidget::onBtnExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), tr(""), tr("Json File(*.json)"));
    if (!fileName.isEmpty()) {
        d->model->exportExternalJson(fileName);
    }
}

void ShortcutSettingWidget::onBtnRecordClicked()
{
    bool bRet = d->editShortCut->isHotkeyMode();
    if (bRet) {
        d->btnRecord->setText("Record");
        d->editShortCut->setHotkeyMode(false);
    } else {
        d->btnRecord->setText("Stop Recording");
        d->editShortCut->setHotkeyMode(true);
    }
}
