// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchresultwidget.h"
#include "searchresultmodel.h"
#include "searchresultitemdelegate.h"

#include "common/util/eventdefinitions.h"

#include <DTreeView>
#include <DLabel>

#include <QMenu>
#include <QVBoxLayout>
#include <QClipboard>

DWIDGET_USE_NAMESPACE

class SearchResultWidgetPrivate : public QObject
{
public:
    explicit SearchResultWidgetPrivate(SearchResultWidget *qq);

    void initUI();
    void initConnection();

    void openFile(const QModelIndex &index);
    void handleMenuRequested(const QPoint &pos);
    void updateMessage();
    void readyReplace(const QModelIndex &index);

public:
    SearchResultWidget *q;

    DLabel *msgLabel { nullptr };
    DTreeView *resultView { nullptr };
    SearchResultModel resultModel;
};

SearchResultWidgetPrivate::SearchResultWidgetPrivate(SearchResultWidget *qq)
    : q(qq)
{
}

void SearchResultWidgetPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    msgLabel = new DLabel(q);
    msgLabel->setWordWrap(true);

    resultView = new DTreeView(q);
    resultView->setModel(&resultModel);
    resultView->setItemDelegate(new SearchResultItemDelegate(resultView));
    resultView->setContextMenuPolicy(Qt::CustomContextMenu);
    resultView->setHeaderHidden(true);
    resultView->setFrameShape(QFrame::NoFrame);
    resultView->setIconSize({ 16, 16 });
    resultView->setIndentation(0);

    mainLayout->addWidget(msgLabel);
    mainLayout->addWidget(resultView);
}

void SearchResultWidgetPrivate::initConnection()
{
    connect(resultView, &DTreeView::doubleClicked, this, &SearchResultWidgetPrivate::openFile);
    connect(resultView, &DTreeView::customContextMenuRequested, this, &SearchResultWidgetPrivate::handleMenuRequested);
    connect(&resultModel, &SearchResultModel::rowsInserted, this, &SearchResultWidgetPrivate::updateMessage);
    connect(&resultModel, &SearchResultModel::rowsRemoved, this, &SearchResultWidgetPrivate::updateMessage);
    connect(&resultModel, &SearchResultModel::modelReset, this, &SearchResultWidgetPrivate::updateMessage);
    connect(&resultModel, &SearchResultModel::requestReplace, this, &SearchResultWidgetPrivate::readyReplace);
}

void SearchResultWidgetPrivate::openFile(const QModelIndex &index)
{
    auto item = resultModel.findItem(index);
    if (!item)
        return;

    editor.gotoPosition(item->filePathName, item->line, item->column);
}

void SearchResultWidgetPrivate::handleMenuRequested(const QPoint &pos)
{
    QModelIndex index = resultView->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu;
    if (resultModel.hasChildren(index)) {
        menu.addAction(SearchResultWidget::tr("Replace All"), this, std::bind(&SearchResultWidgetPrivate::readyReplace, this, index));
        menu.addAction(SearchResultWidget::tr("Dismiss"), &resultModel, std::bind(&SearchResultModel::remove, &resultModel, index));
        menu.addSeparator();
        menu.addAction(SearchResultWidget::tr("Copy Path"), this,
                       [index] {
                           const auto &path = index.data(FilePathRole).toString();
                           QGuiApplication::clipboard()->setText(path);
                       });
    } else {
        menu.addAction(SearchResultWidget::tr("Replace"), this, std::bind(&SearchResultWidgetPrivate::readyReplace, this, index));
        menu.addAction(SearchResultWidget::tr("Dismiss"), &resultModel, std::bind(&SearchResultModel::remove, &resultModel, index));
    }

    menu.exec(QCursor::pos());
}

void SearchResultWidgetPrivate::updateMessage()
{
    Q_EMIT q->resultCountChanged();
    const auto &result = resultModel.allResult();
    if (result.isEmpty()) {
        msgLabel->clear();
        return;
    }

    int totalResult = 0;
    for (const auto &list : result) {
        totalResult += list.size();
    }

    QString msg;
    if (result.count() == 1 && result.first().count() == 1) {
        msg = tr("<font color='blue'>1</font> result in <font color='blue'>1</font> file");
    } else if (result.count() == 1) {
        msg = tr("<font color='blue'>%1</font> results in <font color='blue'>1</font> file").arg(totalResult);
    } else {
        msg = tr("<font color='blue'>%1</font> results in <font color='blue'>%2</font> files").arg(totalResult).arg(result.count());
    }

    msgLabel->setText(msg);
}

void SearchResultWidgetPrivate::readyReplace(const QModelIndex &index)
{
    const auto &result = resultModel.findResult(index);
    resultModel.remove(index);
    Q_EMIT q->requestReplace(result);
}

SearchResultWidget::SearchResultWidget(QWidget *parent)
    : QWidget(parent),
      d(new SearchResultWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

SearchResultWidget::~SearchResultWidget()
{
    delete d;
}

void SearchResultWidget::clear()
{
    d->resultModel.clear();
}

void SearchResultWidget::setReplaceText(const QString &text, bool regex)
{
    d->resultModel.setReplaceText(text, regex);
    d->resultView->viewport()->update();
}

void SearchResultWidget::appendResults(const FindItemList &itemList)
{
    d->resultModel.appendResult(itemList);
}

QMap<QString, FindItemList> SearchResultWidget::allResult() const
{
    return d->resultModel.allResult();
}

bool SearchResultWidget::isEmpty() const
{
    return d->resultModel.rowCount() == 0;
}

void SearchResultWidget::expandAll()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->resultView->expandAll();
    QApplication::restoreOverrideCursor();
}

void SearchResultWidget::collapseAll()
{
    d->resultView->collapseAll();
}

void SearchResultWidget::showMessage(const QString &msg, MessageType type)
{
    switch (type) {
    case Information:
        d->msgLabel->setForegroundRole(DPalette::TextTips);
        break;
    case Warning:
        d->msgLabel->setForegroundRole(DPalette::TextWarning);
    default:
        break;
    }

    d->msgLabel->setText(msg);
}
