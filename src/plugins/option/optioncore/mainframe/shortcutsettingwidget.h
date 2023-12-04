// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTSETTINGWIDGET_H
#define SHORTCUTSETTINGWIDGET_H

#include <DWidget>
#include <DFrame>
#include <QAbstractTableModel>
#include <common/widget/pagewidget.h>

enum ColumnID
{

    kDescriptions,
    kShortcut,
    _KCount
};

class ShortCutPrivate;
class ShortCut : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit ShortCut(QWidget *parent = nullptr);
    virtual ~ShortCut();

    int rowCount() const;
    int columnCount() const;

    void updateUi();
    void updateShortcut(QString id, QString shortcut);
    void resetAllShortcut();
    void saveShortcut();
    void readShortcut();
    void importExternalJson(const QString &filePath);
    void exportExternalJson(const QString &filePath);
    bool shortcutRepeat(const QString &text) const;
    bool keySequenceIsInvalid(const QKeySequence &sequence) const;

signals:

private:
    ShortCutPrivate *d;
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

signals:

public slots:
    void onBtnResetAllClicked();
    void onBtnImportClicked();
    void onBtnExportClicked();
private:
    void setupUi();
    ShortcutSettingWidgetPrivate *const d;
};

#endif // SHORTCUTSETTINGWIDGET_H
