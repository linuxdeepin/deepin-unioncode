// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGECONTROLCOMPONENT_H
#define PAGECONTROLCOMPONENT_H

#include <DWidget>

//DWIDGET_BEGIN_NAMESPACE
//class DPushButton;
//DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QHBoxLayout;
QT_END_NAMESPACE

class PageControlComponent : public DWidget
{
    Q_OBJECT
public:
    explicit PageControlComponent(QWidget *parent = nullptr);

    void updatePageControl(int count);
    int currentIndex() const;

public Q_SLOTS:
    void onPreBtnClicked();
    void onNextBtnClicked();
    void onLeftMoreBtnClicked();
    void onRightMoreBtnClicked();
    void onNumberBtnClicked();

Q_SIGNALS:
    void pageChanged(int pageIndex);

private:
    void initUI();
    void initPageButtons();
    void initConnection();
    void updatePageButtons();

    void setCurrentPage(int pageIndex);

    DPushButton *createPushButton();

    inline int indexToNumber(int index) { return index + 1; }

    DPushButton *preButton { nullptr };
    DPushButton *nextButton { nullptr };
    DPushButton *firstPageButton { nullptr };
    DPushButton *lastPageButton { nullptr };
    DPushButton *leftMoreButton { nullptr };
    DPushButton *rightMoreButton { nullptr };
    QList<DPushButton *> midPageButtonsList {};

    QHBoxLayout *pageButtonsLayout { nullptr };

    int pageCount { 1 };
    int currentPageIndex { 0 };
};

#endif // PAGECONTROLCOMPONENT_H
