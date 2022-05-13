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

#ifndef SHORTCUTSETTINGWIDGET_H
#define SHORTCUTSETTINGWIDGET_H

#include "common/widget/hotkeylineedit.h"

#include <QWidget>
#include <QKeySequence>
#include <QTreeWidgetItem>
#include <QPushButton>

struct ShortcutItem
{
    QString m_cmd;
    QKeySequence m_key;
    QTreeWidgetItem *m_item;
};


enum ColumnID
{
    kID,
    kDescriptions,
    kShortcut,
    _KCount
};

class ShortcutTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ShortcutTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void updateShortcut(QString qsID, QString qsShortcut);
    void resetShortcut(QString qsID);
    void resetAllShortcut();
    void saveShortcut();
    bool readFromJson(const QString &qsFilePath, QMap<QString, QStringList> &mapShortcutItem);
    bool writeToJson(const QString &qsFilePath, const QMap<QString, QStringList> &mapShortcutItem);
    void importExternalJson(const QString &qsFilePath);
    void exportExternalJson(const QString &qsFilePath);

private:
    QMap<QString, QStringList> m_mapShortcutItem;
    QMap<QString, QStringList> m_mapShortcutItemShadow;
    QString m_qsConfigFilePath;
};


class QTableView;
class QLineEdit;
class ShortcutSettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutSettingWidget(QWidget *parent = nullptr);
    void saveConfig();

signals:

public slots:
    void onTableViewClicked(const QModelIndex &);
    void onShortcutEditTextChanged(const QString &);
    void onBtnResetAllClicked();
    void onBtnImportClicked();
    void onBtnExportClicked();
    void onBtnResetClicked();
    void onBtnRecordClicked();
private:
    void setupUi();
    void setSelectedShortcut();

    QTableView *m_tableView;
    HotkeyLineEdit *m_editShortCut;
    ShortcutTableModel *m_model;
    QPushButton *m_btnRecord;
};

#endif // SHORTCUTSETTINGWIDGET_H
