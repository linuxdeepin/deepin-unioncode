// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTSETTINGWIDGET_H
#define SHORTCUTSETTINGWIDGET_H

#include <DWidget>
#include <QAbstractTableModel>
#include <common/widget/pagewidget.h>

enum ColumnID
{
    kID,
    kDescriptions,
    kShortcut,
    _KCount
};

class ShortcutTableModelPrivate;
class ShortcutTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ShortcutTableModel(QObject *parent = nullptr);
    virtual ~ShortcutTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void updateShortcut(QString id, QString shortcut);
    void resetShortcut(QString id);
    void resetAllShortcut();
    void saveShortcut();
    void readShortcut();
    void importExternalJson(const QString &filePath);
    void exportExternalJson(const QString &filePath);
    bool shortcutRepeat(const QString &text) const;
    bool keySequenceIsInvalid(const QKeySequence &sequence) const;

signals:

private:
    ShortcutTableModelPrivate *d;
};

class ShortcutSettingWidgetPrivate;
class ShortcutSettingWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit ShortcutSettingWidget(QWidget *parent = nullptr);
    virtual ~ShortcutSettingWidget();
    void saveConfig();
    void readConfig();
    void checkShortcutValidity(const int row, const QString &shortcut);
    bool shortcutIsRepeat(const int row, const QString &text);

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
    ShortcutSettingWidgetPrivate *const d;
};

#endif // SHORTCUTSETTINGWIDGET_H
