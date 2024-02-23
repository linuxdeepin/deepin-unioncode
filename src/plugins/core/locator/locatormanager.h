// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCATORMANAGER_H
#define LOCATORMANAGER_H

#include "popupwidget.h"

#include <DSearchEdit>
#include <DWidget>
#include <DSpinner>

#include <QObject>
#include <QtConcurrent>
#include <QShortcut>
#include <QKeySequence>

class AllLocators : public abstractLocator
{
public:
    AllLocators(QObject *parent = nullptr);

    void prepareSearch(const QString &searchText) override;
    QList<baseLocatorItem> matchesFor(const QString &inputText) override;
    void accept(baseLocatorItem item) override;

private:
    QList<baseLocatorItem> itemList;
};

class LocatorManager : public QObject
{
    Q_OBJECT
public:
    explicit LocatorManager(QObject *parent = nullptr);
    ~LocatorManager();
    static LocatorManager *instance();

    PopupWidget *getPopupWidget() { return popupWidget; }
    DTK_WIDGET_NAMESPACE::DSearchEdit *getInputEdit() { return inputEdit; }
    locatorModel *getModel() { return model; }
    QList<abstractLocator *> getLocatorlist() { return locatorList; }

    void registerLocator(abstractLocator *locator);
    QList<abstractLocator *> getValidLocator(const QString &text, QString &searchText);
    QList<baseLocatorItem> runSearch(QList<abstractLocator *> validLocator, const QString &searchText);
    void updatePopupWidget(const QString &text);

    void accept(const QModelIndex &index);
signals:
    void handleKey(QKeyEvent *keyEvent);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool searchAgain { false };
    void initConnect();
    void initService();
    void showSpinner();
    void initShortCut();
    void setShortCutForLocator(abstractLocator *locator, const QKeySequence &key);

    locatorModel *model { nullptr };
    PopupWidget *popupWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DSearchEdit *inputEdit { nullptr };

    DSpinner *spinner { nullptr };

    QFutureWatcher<void> watcher;

    QList<abstractLocator *> locatorList;
    QShortcut *shortCut { nullptr };

    QTimer timer;
};

#endif   // LOCATORMANAGER_H
