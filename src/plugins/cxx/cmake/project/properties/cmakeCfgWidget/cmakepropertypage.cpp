// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakepropertypage.h"
#include "cmakeitemmodel.h"
#include "cmakeitemdelegate.h"

#include <DTableView>
#include <DHeaderView>

#include <QVBoxLayout>
#include <QFile>
#include <QByteArrayList>
#include <QDebug>

DWIDGET_USE_NAMESPACE

static QByteArrayList splitCMakeCacheLine(const QByteArray &line)
{
    const int colonPos = line.indexOf(':');
    if (colonPos < 0)
        return QByteArrayList();

    const int equalPos = line.indexOf('=', colonPos + 1);
    if (equalPos < colonPos)
        return QByteArrayList();

    return QByteArrayList() << line.mid(0, colonPos)
                            << line.mid(colonPos + 1, equalPos - colonPos - 1)
                            << line.mid(equalPos + 1);
}

class CMakePropertyPagePrivate
{
    friend class CMakePropertyPage;

    DTableView *tableView { nullptr };
    CMakeItemModel *model { nullptr };
};

CMakePropertyPage::CMakePropertyPage(QWidget *parent)
    : DWidget(parent), d(new CMakePropertyPagePrivate)
{
    setupUi();
}

CMakePropertyPage::~CMakePropertyPage()
{
    if (d)
        delete d;
}

void CMakePropertyPage::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    d->tableView = new DTableView(this);
    d->tableView->setShowGrid(false);
    d->tableView->setAlternatingRowColors(true);
    d->tableView->setFrameShape(QFrame::NoFrame);
    d->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->tableView->sortByColumn(0, Qt::AscendingOrder);
    DHeaderView* headerView = d->tableView->horizontalHeader();
    headerView->setStretchLastSection(true);
    headerView->setSectionResizeMode(DHeaderView::Interactive);
    headerView->setDefaultAlignment(Qt::AlignLeft);
    headerView->setDefaultSectionSize(200);

    d->model = new CMakeItemModel(this);
    d->tableView->setModel(d->model);
    d->tableView->setEditTriggers(QAbstractItemView::DoubleClicked);
    d->tableView->setItemDelegate(new CMakeItemDelegate(this));

    vLayout->addWidget(d->tableView);
}

void CMakePropertyPage::saveConfigToCacheFile(const QString &cacheFile)
{
    QFile cache(cacheFile);
    if (!cache.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CMakeCache File Open Failed!";
        return;
    }

    QFile tempFile(cacheFile + ".tmp");
    if (!tempFile.open(QFile::ReadWrite)) {
        cache.close();
        qWarning() << "Temporary File Creation Failed!";
        return;
    }

    QTextStream in(&cache);
    QTextStream out(&tempFile);

    auto items = d->model->getItems();
    //get items need to chang
    QMap<QString, CMakeItem> itemsNeedToChange;
    for (auto item : items) {
        if (item.isChanged) {
            itemsNeedToChange.insert(item.key, item);
        }
    }

    if (itemsNeedToChange.isEmpty())
        return;

    QString line;
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith("//")) {
            out << line << '\n';
            continue;
        }

        auto splitResult = splitCMakeCacheLine(line.toUtf8());
        if (splitResult.count() != 3)
            continue;

        auto key = splitResult.at(0);

        if (itemsNeedToChange.keys().contains(key)) {
            auto item = itemsNeedToChange[key];
            QString newLine = item.key + ':' + CMakeItem::typeToString(item.type) + '=' + item.value.toString();
            out << newLine << '\n';
        } else {
            out << line << '\n';
        }
    }

    cache.close();
    tempFile.close();

    // replace cmakeCache
    if (!QFile::remove(cacheFile)) {
        qWarning() << "Failed to remove original file!";
        return;
    }
    if (!tempFile.rename(cacheFile)) {
        qWarning() << "Failed to rename temporary file!";
        return;
    }
    emit cacheFileUpdated();
}

void CMakePropertyPage::getItemsFromCacheFile(const QString &cacheFile)
{
    QFile cache(cacheFile);
    if (!cache.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CMakeCache File Open Failed!";
        return;
    }

    QString description;
    QList<CMakeItem> items;
    QStringList advancedKey;
    while (!cache.atEnd()) {
        const QByteArray line = cache.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#'))
            continue;

        if (line.startsWith("//")) {
            description = line.mid(2);
            continue;
        }

        auto splitResult = splitCMakeCacheLine(line);
        if (splitResult.count() != 3)
            continue;

        auto key = splitResult.at(0);
        auto type = splitResult.at(1);
        auto value = splitResult.at(2);

        CMakeItem item;
        // todo : process key endsWith(-STRINGS)
        if (key.endsWith("-ADVANCED") && value == "1") {
            item.key = key.left(key.count() - 9);
            advancedKey.append(item.key);
            continue;
        }

        item.key = key;
        item.value = value;
        if (value.isEmpty())
            item.isUnset = true;
        item.type = CMakeItem::dataToType(type);
        item.description = description;
        item.isInCacheFile = true;

        items.append(item);
    }

    //set advanced
    for (auto &item : items) {
        if (advancedKey.contains(item.key))
            item.isAdvanced = true;
    }

    d->model->setData(items);
}

