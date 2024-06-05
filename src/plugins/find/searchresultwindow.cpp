// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchresultwindow.h"
#include "common/common.h"
#include "qobjectdefs.h"
#include "base/baseitemdelegate.h"

#include <DPushButton>
#include <DMessageBox>
#include <DLabel>
#include <DLineEdit>
#include <DIconButton>
#include <DDialog>

#include <QVBoxLayout>
#include <QDebug>
#include <QPalette>

SearchResultModel::SearchResultModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int SearchResultModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    if (auto item = findItem(index))
        return data(*item, role);

    QString group = findGroup(index);
    if (!group.isEmpty())
        return data(group, role);

    return QVariant();
}

QModelIndex SearchResultModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column == 0 && parent.isValid()) {
        QString group = findGroup(parent);
        if (!group.isEmpty()) {
            auto items = resultData.value(group);
            if (row < items.count()) {
                auto &item = items.at(row);
                return createIndex(row, 0, const_cast<FindItem *>(&item));
            }
        }
    } else if (column == 0 && row < resultData.size()) {
        return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex SearchResultModel::parent(const QModelIndex &child) const
{
    if (auto tool = findItem(child)) {
        int groupIndex = 0;
        for (const auto &itemsInGroup : resultData) {
            if (itemsInGroup.contains(*tool))
                return index(groupIndex, 0);
            ++groupIndex;
        }
    }
    return QModelIndex();
}

int SearchResultModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return resultData.size();

    if (findItem(parent))
        return 0;

    QString group = findGroup(parent);
    if (!group.isEmpty())
        return resultData.value(group).count();

    return 0;
}

Qt::ItemFlags SearchResultModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void SearchResultModel::clear()
{
    beginResetModel();
    resultData.clear();
    endResetModel();
}

FindItem *SearchResultModel::findItem(const QModelIndex &index) const
{
    auto item = static_cast<FindItem *>(index.internalPointer());
    return item;
}

QString SearchResultModel::findGroup(const QModelIndex &index) const
{
    if (index.isValid() && !index.parent().isValid() && index.column() == 0 && index.row() >= 0) {
        const QList<QString> &keys = resultData.keys();
        if (index.row() < keys.count())
            return keys.at(index.row());
    }

    return QString();
}

void SearchResultModel::appendResult(const FindItemList &list)
{
    QMap<QString, FindItemList> result;
    for (const auto &item : list) {
        if (!resultData.contains(item.filePathName)) {
            if (result.contains(item.filePathName)) {
                addItem(item.filePathName, result[item.filePathName]);
                result.clear();
            }
            addGroup(item.filePathName);
            result[item.filePathName].append(item);
        } else {
            result[item.filePathName].append(item);
        }
    }

    for (auto iter = result.begin(); iter != result.end(); ++iter)
        addItem(iter.key(), iter.value());
}

void SearchResultModel::addGroup(const QString &group)
{
    QList<QString> groupList = resultData.keys();
    groupList.append(group);
    std::stable_sort(std::begin(groupList), std::end(groupList));
    int pos = groupList.indexOf(group);

    beginInsertRows(QModelIndex(), pos, pos);
    resultData.insert(group, FindItemList());
    endInsertRows();
}

void SearchResultModel::addItem(const QString &group, const FindItemList &itemList)
{
    int pos = resultData.keys().indexOf(group);
    auto parent = index(pos, 0);

    beginInsertRows(parent, pos, pos + itemList.size());
    resultData[group].append(itemList);
    endInsertRows();
}

QVariant SearchResultModel::data(const FindItem &item, int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
        return item.context;
    case Qt::DisplayRole:
        return QString::number(item.lineNumber) + "   " + item.context;
    default:
        break;
    }

    return QVariant();
}

QVariant SearchResultModel::data(const QString &group, int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
        return group;
    case Qt::DisplayRole: {
        QString format("%1 (%2)");
        return format.arg(group).arg(resultData.value(group).count());
    }
    case Qt::DecorationRole:
        return DFileIconProvider::globalProvider()->icon(QFileInfo(group));
    default:
        break;
    }

    return QVariant();
}

class SearchResultTreeViewPrivate
{
    SearchResultTreeViewPrivate() {}
    ~SearchResultTreeViewPrivate();

    SearchResultModel model;
    friend class SearchResultTreeView;
};

SearchResultTreeViewPrivate::~SearchResultTreeViewPrivate()
{
}

SearchResultTreeView::SearchResultTreeView(QWidget *parent)
    : DTreeView(parent), d(new SearchResultTreeViewPrivate())
{
    setModel(&d->model);

    connect(this, &DTreeView::doubleClicked, this, [=](const QModelIndex &index) {
        auto item = d->model.findItem(index);
        if (!item)
            return;

        editor.gotoLine(item->filePathName, item->lineNumber);
    });
}

SearchResultTreeView::~SearchResultTreeView()
{
    delete d;
}

void SearchResultTreeView::appendData(const FindItemList &itemList)
{
    d->model.appendResult(itemList);
}

QIcon SearchResultTreeView::icon(const QString &data)
{
    QFileInfo info(data);
    return iconProvider.icon(info);
}

void SearchResultTreeView::clearData()
{
    d->model.clear();
}

class SearchResultWindowPrivate
{
    SearchResultWindowPrivate() {}
    SearchResultTreeView *treeView { nullptr };
    QWidget *replaceWidget { nullptr };
    DLineEdit *replaceEdit { nullptr };
    DLabel *resultLabel { nullptr };
    QLabel *iconLabel { nullptr };
    DDialog *replaceTextDialog { nullptr };
    DDialog *replaceWarningDialog { nullptr };
    DPushButton *replaceBtn { nullptr };
    bool replaceTextFlag = false;
    SearchParams searchParams;
    int resultCount { 0 };

    friend class SearchResultWindow;
};

SearchResultWindow::SearchResultWindow(QWidget *parent)
    : QWidget(parent), d(new SearchResultWindowPrivate())
{
    setupUi();

    qRegisterMetaType<FindItemList>("FindItemList");
}

SearchResultWindow::~SearchResultWindow()
{
    delete d;
}

void SearchResultWindow::clear()
{
    d->iconLabel->setVisible(true);
    d->treeView->clearData();
}

void SearchResultWindow::setupUi()
{
    d->replaceWidget = new QWidget(this);
    QHBoxLayout *replaceLayout = new QHBoxLayout();

    d->replaceEdit = new DLineEdit(this);
    d->replaceEdit->setFixedWidth(280);
    d->replaceEdit->setPlaceholderText(tr("Replace"));
    d->replaceBtn = new DPushButton(DPushButton::tr("Replace"), this);
    d->replaceBtn->setFixedSize(120, 36);
    d->replaceWidget->setLayout(replaceLayout);

    replaceLayout->addWidget(d->replaceEdit, 0, Qt::AlignLeft);
    replaceLayout->addWidget(d->replaceBtn, 0, Qt::AlignLeft);
    replaceLayout->setAlignment(Qt::AlignLeft);

    QHBoxLayout *hLayout = new QHBoxLayout();
    DIconButton *cleanBtn = new DIconButton(this);   //Clean && Return
    cleanBtn->setIcon(QIcon::fromTheme("go-previous"));
    QSize iconSize(12, 12);
    cleanBtn->setIconSize(iconSize);

    cleanBtn->setFixedSize(36, 36);
    d->resultLabel = new DLabel(this);

    hLayout->addWidget(cleanBtn);
    hLayout->addWidget(d->replaceWidget);
    hLayout->addWidget(d->resultLabel);
    hLayout->setAlignment(Qt::AlignLeft);

    d->treeView = new SearchResultTreeView(this);
    d->treeView->setHeaderHidden(true);
    d->treeView->setLineWidth(0);
    d->treeView->setItemDelegate(new BaseItemDelegate(this));

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(d->treeView, 1);

    d->iconLabel = new QLabel(this);
    QVBoxLayout *iconLayout = new QVBoxLayout();
    d->iconLabel->setPixmap(QIcon::fromTheme("find_noResults").pixmap(QSize(96, 96)));
    iconLayout->addWidget(d->iconLabel, Qt::AlignCenter);

    iconLayout->setAlignment(Qt::AlignCenter);
    vLayout->addLayout(iconLayout);
    vLayout->setAlignment(iconLayout, Qt::AlignCenter);

    connect(cleanBtn, &DIconButton::clicked, this, &SearchResultWindow::clean);
    connect(d->replaceBtn, &DPushButton::clicked, this, &SearchResultWindow::replace);

    setLayout(vLayout);
    setRepalceWidgtVisible(false);
}

void SearchResultWindow::setRepalceWidgtVisible(bool visible)
{
    d->replaceBtn->setEnabled(!visible);
    d->replaceWidget->setVisible(visible);
}

void SearchResultWindow::appendResults(const FindItemList &itemList)
{
    d->treeView->setVisible(true);
    d->iconLabel->setVisible(false);
    d->treeView->appendData(itemList);
    d->resultCount += itemList.count();
    QString msg = tr("%1 matches found.").arg(d->resultCount);
    showMsg(true, msg);
}

void SearchResultWindow::searchFinished()
{
    if (d->resultCount > 0) {
        QString msg = tr("Search completed, %1 matches found.").arg(d->resultCount);
        showMsg(true, msg);
        d->replaceBtn->setEnabled(true);
        return;
    }

    d->treeView->setVisible(false);
    d->iconLabel->setVisible(true);
    showMsg(false, tr("No match found!"));
}

void SearchResultWindow::replaceFinished(bool success)
{
    QString msg = success ? tr("Replacement successful!") : tr("Replace failed!");
    showMsg(success, msg);
}

void SearchResultWindow::clean()
{
    d->resultCount = 0;
    d->treeView->clearData();
    emit reqBack();
}

void SearchResultWindow::replace()
{
    showMsg(true, tr("Replacing, please wait..."));
    QString replaceText = d->replaceEdit->text();
    if (replaceText.isEmpty()) {
        d->replaceTextFlag = false;
        d->replaceTextDialog = new DDialog(this);
        d->replaceTextDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        d->replaceTextDialog->setMessage(tr("Repalce text is empty, will continue?"));
        d->replaceTextDialog->insertButton(0, tr("No"));
        d->replaceTextDialog->insertButton(1, tr("Yes"), true, DDialog::ButtonRecommend);

        connect(d->replaceTextDialog, &DDialog::buttonClicked, this, [=](int index) {
            if (index == 0) {
                d->replaceTextFlag = true;
                d->replaceTextDialog->reject();
            } else if (index == 1) {
                d->replaceTextDialog->accept();
            }
        });
        d->replaceTextDialog->exec();
    }

    if (d->replaceTextFlag)
        return;

    Q_EMIT reqReplace(replaceText);
}

void SearchResultWindow::showMsg(bool succeed, QString msg)
{
    QPalette palette = d->resultLabel->palette();
    QColor textColor = this->palette().color(QPalette::WindowText);

    int red, green, blue, alpha;
    textColor.getRgb(&red, &green, &blue, &alpha);
    // 降低透明度
    alpha = static_cast<int>(alpha * 0.5);   // 0.5 表示减少50%

    QColor newColor = QColor::fromRgb(red, green, blue, alpha);

    palette.setColor(QPalette::WindowText, newColor);
    d->resultLabel->setPalette(palette);
    d->resultLabel->setText(msg);
}
