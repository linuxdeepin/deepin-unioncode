// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "combinationcomboxbox.h"
#include "models/binarytoolsmodel.h"

#include <DSearchEdit>

#include <QTreeView>
#include <QHeaderView>
#include <QStylePainter>
#include <QApplication>
#include <QScreen>

DWIDGET_USE_NAMESPACE

class CompleterViewPrivate
{
public:
    explicit CompleterViewPrivate(CompleterView *qq);

    void initUI();
    void initConnections();

public:
    CompleterView *q;

    DSearchEdit *searchEdit { nullptr };
    QTreeView *treeView { nullptr };
    BinaryToolsModel model;
};

CompleterViewPrivate::CompleterViewPrivate(CompleterView *qq)
    : q(qq)
{
}

void CompleterViewPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    searchEdit = new DSearchEdit(q);
    treeView = new QTreeView(q);
    treeView->setFrameShape(QFrame::NoFrame);
    treeView->setDragEnabled(false);
    treeView->header()->setVisible(false);
    treeView->setModel(&model);

    mainLayout->addWidget(searchEdit);
    mainLayout->addWidget(treeView);
}

void CompleterViewPrivate::initConnections()
{
}

CompleterView::CompleterView(QWidget *parent)
    : QFrame(parent, Qt::Popup),
      d(new CompleterViewPrivate(this))
{
    d->initUI();
    d->initConnections();
}

CompleterView::~CompleterView()
{
    delete d;
}

void CompleterView::updateTools(const QMap<QString, QList<ToolInfo>> &tools)
{
    d->model.setTools(tools);
}

void CompleterView::showEvent(QShowEvent *e)
{
    d->treeView->expandAll();
    QFrame::showEvent(e);
}

CombinationComboxBox::CombinationComboxBox(QWidget *parent)
    : QComboBox(parent),
      completerView(new CompleterView(this))
{
}

void CombinationComboxBox::setTools(const QMap<QString, QList<ToolInfo> > &tools)
{
    completerView->updateTools(tools);
}

void CombinationComboxBox::setText(const QString &text)
{
}

QString CombinationComboxBox::text()
{
    return {};
}

void CombinationComboxBox::clear()
{
}

void CombinationComboxBox::showPopup()
{
    QStyle *const style = this->style();
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    QRect rect(style->subControlRect(QStyle::CC_ComboBox, &opt,
                                     QStyle::SC_ComboBoxListBoxPopup, this));
    QPoint below = mapToGlobal(rect.bottomLeft());
    below.setY(below.y() + 4);

    completerView->setFixedWidth(width());
    completerView->show();
    completerView->move(below);
}

void CombinationComboxBox::paintEvent(QPaintEvent *event)
{
    QComboBox::paintEvent(event);
}
