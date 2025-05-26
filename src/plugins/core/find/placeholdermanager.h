// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLACEHOLDERMANAGER_H
#define PLACEHOLDERMANAGER_H

#include <QWidget>

class FindToolBar;
class AbstractDocumentFind;
class FindToolBarPlaceHolder : public QWidget
{
    Q_OBJECT
public:
    explicit FindToolBarPlaceHolder(QWidget *owner, QWidget *parent = nullptr);
    ~FindToolBarPlaceHolder() override;

    QWidget *getOwner() const;
    bool isUsedByWidget(QWidget *widget);
    void setWidget(FindToolBar *widget);

private:
    QWidget *owner { nullptr };
    FindToolBar *findToolBar { nullptr };
};

class PlaceHolderManager
{
public:
    static PlaceHolderManager *instance();

    FindToolBarPlaceHolder *getCurrentHolder();
    void setCurrentHolder(FindToolBarPlaceHolder *holder);

    QWidget *createPlaceHolder(QWidget *owner, AbstractDocumentFind *docFind);
    void removeHolder(FindToolBarPlaceHolder *holder);
    AbstractDocumentFind *findDocumentFind(FindToolBarPlaceHolder *holder) const;

    using HolderInfo = QHash<FindToolBarPlaceHolder *, AbstractDocumentFind *>;
    const HolderInfo allHolderInfo();

private:
    explicit PlaceHolderManager() = default;

    HolderInfo holderInfo;
    FindToolBarPlaceHolder *currentHolder { nullptr };
};

#endif   // PLACEHOLDERMANAGER_H
