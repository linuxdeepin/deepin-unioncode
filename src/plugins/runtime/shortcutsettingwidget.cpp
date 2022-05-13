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
#include <QFile>
#include <QJsonParseError>
#include <QJsonObject>
#include <QDebug>
#include <QDir>
#include <QFileDialog>

#define BTN_WIDTH (180)

ShortcutTableModel::ShortcutTableModel(QObject *parent) :
    QAbstractTableModel(parent)
  , m_qsConfigFilePath(CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("shortcut.support"))
{

    bool bRet = readFromJson(m_qsConfigFilePath, m_mapShortcutItem);
    if (!bRet) {
        qInfo() << "Read shortcut setting error!" << endl;
    }
    m_mapShortcutItemShadow = m_mapShortcutItem;
}

int ShortcutTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_mapShortcutItem.size();
}

int ShortcutTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return ColumnID::_KCount;
}

QVariant ShortcutTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.row() >= m_mapShortcutItem.keys().size())
        return QVariant();

    QString qsID = m_mapShortcutItem.keys()[index.row()];
    QStringList qsListValue = m_mapShortcutItem[qsID];

    QString qsDesc = qsListValue.first();
    QString qsShortcut = qsListValue.last();

    switch (index.column()) {
        case ColumnID::kID:
            return qsID;
        case ColumnID::kDescriptions:
            return qsDesc;
        case ColumnID::kShortcut:
            return qsShortcut;
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

void ShortcutTableModel::updateShortcut(QString qsID, QString qsShortcut)
{
    if (m_mapShortcutItem.keys().contains(qsID))
    {
        QStringList qsListValue = m_mapShortcutItem.value(qsID);
        QStringList qsNewListValue = {qsListValue.first(), qsShortcut};
        m_mapShortcutItem[qsID] = qsNewListValue;
    }
}

void ShortcutTableModel::resetShortcut(QString qsID)
{
    if (m_mapShortcutItem.keys().contains(qsID) && m_mapShortcutItemShadow.keys().contains(qsID))
    {
        QStringList qsListShadowValue = m_mapShortcutItemShadow.value(qsID);
        m_mapShortcutItem[qsID] = qsListShadowValue;
    }
}

void ShortcutTableModel::resetAllShortcut()
{
    m_mapShortcutItem = m_mapShortcutItemShadow;
}

void ShortcutTableModel::saveShortcut()
{
    writeToJson(m_qsConfigFilePath, m_mapShortcutItem);
}

bool ShortcutTableModel::readFromJson(const QString &qsFilePath, QMap<QString, QStringList> &mapShortcutItem)
{
    Q_UNUSED(mapShortcutItem)
    QFile file(qsFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        return false;
    }

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();
    mapShortcutItem.clear();
    for (auto key : rootObject.keys()) {
        if (!rootObject.value(key).isArray())
            continue;
        QJsonArray valueArray = rootObject.value(key).toArray();
        if (valueArray.count() < 2)
            continue;

        QStringList qsListValue = {valueArray.first().toString(), valueArray.last().toString()};
        mapShortcutItem[key] = qsListValue;
    }

    return true;
}

bool ShortcutTableModel::writeToJson(const QString &qsFilePath, const QMap<QString, QStringList> &mapShortcutItem)
{
    Q_UNUSED(mapShortcutItem)

    QJsonObject rootObject;
    QMap<QString, QStringList>::const_iterator iter = mapShortcutItem.begin();
    for (; iter != mapShortcutItem.end(); ++iter)
    {
        QString qsID = iter.key();
        QString qsDesc = iter.value().first();
        QString qsShortcut = iter.value().last();

        QJsonArray valueArray;
        valueArray.append(QJsonValue(qsDesc));
        valueArray.append(QJsonValue(qsShortcut));

        rootObject.insert(qsID, valueArray);
    }

    QJsonDocument doc;
    doc.setObject(rootObject);
    QString jsonStr(doc.toJson(QJsonDocument::Indented));

    QFile file(qsFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(jsonStr.toUtf8());
    file.close();

    return true;
}

void ShortcutTableModel::importExternalJson(const QString &qsFilePath)
{
    readFromJson(qsFilePath, m_mapShortcutItem);
    m_mapShortcutItemShadow = m_mapShortcutItem;
}

void ShortcutTableModel::exportExternalJson(const QString &qsFilePath)
{
    writeToJson(qsFilePath, m_mapShortcutItem);
}

ShortcutSettingWidget::ShortcutSettingWidget(QWidget *parent)
    : QWidget(parent)
    , m_tableView(nullptr)
    , m_editShortCut(nullptr)
    , m_model(nullptr)
{
    setupUi();
}

void ShortcutSettingWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    m_tableView = new QTableView();
    m_tableView->setShowGrid(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_model = new ShortcutTableModel();
    m_tableView->setModel(m_model);
    vLayout->addWidget(m_tableView);

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
    m_editShortCut = new HotkeyLineEdit();
    m_btnRecord = new QPushButton("Record");
    m_btnRecord->setFixedWidth(BTN_WIDTH);
    QPushButton *btnReset = new QPushButton("Reset");
    btnReset->setFixedWidth(BTN_WIDTH);
    hLayoutShortcut->addWidget(labelTip);
    hLayoutShortcut->addWidget(m_editShortCut);
    hLayoutShortcut->addWidget(m_btnRecord);
    hLayoutShortcut->addWidget(btnReset);

    connect(m_tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onTableViewClicked(const QModelIndex &)));
    connect(btnResetAll, SIGNAL(clicked()), this, SLOT(onBtnResetAllClicked()));
    connect(m_editShortCut, SIGNAL(textChanged(const QString &)), this, SLOT(onShortcutEditTextChanged(const QString &)));
    connect(btnImport, SIGNAL(clicked()), this, SLOT(onBtnImportClicked()));
    connect(btnExport, SIGNAL(clicked()), this, SLOT(onBtnExportClicked()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(onBtnResetClicked()));
    connect(m_btnRecord, SIGNAL(clicked()), this, SLOT(onBtnRecordClicked()));
}

void ShortcutSettingWidget::setSelectedShortcut()
{
    int row = m_tableView->currentIndex().row();
    QModelIndex index = m_model->index(row, ColumnID::kShortcut);
    QString qsShortcut = m_model->data(index, Qt::DisplayRole).toString();
    m_editShortCut->setText(qsShortcut);
}

void ShortcutSettingWidget::onTableViewClicked(const QModelIndex &)
{
    setSelectedShortcut();
}

void ShortcutSettingWidget::onBtnResetAllClicked()
{
    m_model->resetAllShortcut();
    m_tableView->update();
}

void ShortcutSettingWidget::onBtnResetClicked()
{
    int row = m_tableView->currentIndex().row();
    QModelIndex indexID = m_model->index(row, ColumnID::kID);
    QString qsID = m_model->data(indexID, Qt::DisplayRole).toString();
    m_model->resetShortcut(qsID);

    setSelectedShortcut();
}

void ShortcutSettingWidget::onShortcutEditTextChanged(const QString &text)
{
    int row = m_tableView->currentIndex().row();
    QModelIndex indexID = m_model->index(row, ColumnID::kID);
    QString qsID = m_model->data(indexID, Qt::DisplayRole).toString();
    m_model->updateShortcut(qsID, text);

    QModelIndex indexShortcut = m_model->index(row, ColumnID::kShortcut);
    m_tableView->update(indexShortcut);
}

void ShortcutSettingWidget::saveConfig()
{
    m_model->saveShortcut();
}

void ShortcutSettingWidget::onBtnImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), tr(""), tr("Json File(*.json)"));
    if (!fileName.isEmpty()) {
        m_model->importExternalJson(fileName);
    }
}

void ShortcutSettingWidget::onBtnExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), tr(""), tr("Json File(*.json)"));
    if (!fileName.isEmpty()) {
        m_model->exportExternalJson(fileName);
    }
}

void ShortcutSettingWidget::onBtnRecordClicked()
{
    bool bRet = m_editShortCut->isHotkeyMode();
    if (bRet) {
        m_btnRecord->setText("Start Record");
        m_editShortCut->setHotkeyMode(false);
    } else {
        m_btnRecord->setText("Stop Record");
        m_editShortCut->setHotkeyMode(true);
    }
}
