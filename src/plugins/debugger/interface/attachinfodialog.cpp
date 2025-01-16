// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "attachinfodialog.h"

#include <DLineEdit>
#include <DComboBox>
#include <DHeaderView>
#include <DPushButton>

#include <QFormLayout>
#include <QLabel>
#include <QFile>
#include <QDir>
#include <QDebug>

DWIDGET_USE_NAMESPACE

AttachInfoDialog::AttachInfoDialog(QWidget *parent)
    : DDialog(parent)
{
    initUi();
    initButton();
}

void AttachInfoDialog::initUi()
{
    setFixedSize(500, 500);
    DComboBox *cbBox = new DComboBox(this);
    cbBox->addItem("gdb");
    DLineEdit *edit = new DLineEdit(this);

    auto widget = new DWidget(this);
    auto fLayout = new QFormLayout(widget);
    fLayout->addRow(new QLabel(tr("debugger:"), this), cbBox);
    fLayout->addRow(new QLabel(tr("filter:"), this), edit);

    view = new DTableView(this);
    view->setShowGrid(false);
    view->setAlternatingRowColors(true);
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({tr("Process Id"), tr("Process Path")});
    proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    view->setModel(proxy);

    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->verticalHeader()->hide();
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    setIcon(QIcon::fromTheme("ide"));
    insertContent(0, widget);
    insertContent(1, view);

    connect(edit, &DLineEdit::textChanged, this, [=](const QString &text){
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        proxy->setFilterRegExp(QString(".*%1.*").arg(text));
#else
        proxy->setFilterRegularExpression(QString(".*%1.*").arg(text));
#endif
    });

    updateProcess();
}

void AttachInfoDialog::initButton()
{
    auto buttonBox = new DWidget(this);
    auto buttonLayout = new QHBoxLayout(buttonBox);

    DPushButton *btnUpdate = new DPushButton(tr("Update"), this);
    DPushButton *btnCancel = new DPushButton(tr("Cancel"), this);
    DPushButton *btnAttaching = new DPushButton(tr("Attaching"), this);
    btnAttaching->setEnabled(false);

    buttonLayout->addWidget(btnUpdate);
    buttonLayout->addWidget(btnCancel);
    buttonLayout->addWidget(btnAttaching);

    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](){
        if (view->selectionModel()->hasSelection())
            btnAttaching->setEnabled(true);
        else
            btnAttaching->setEnabled(false);
    });
    connect(btnUpdate, &DPushButton::clicked, this, [=](){
        updateProcess();
    });
    connect(btnCancel, &DPushButton::clicked, this, [=](){
        reject();
    });
    connect(btnAttaching, &DPushButton::clicked, this, [=](){
        auto selectedItem = view->selectionModel()->selectedRows(0).at(0);
        emit attachToProcessId(selectedItem.data().toString());
        accept();
    });

    addContent(buttonBox);
}

void AttachInfoDialog::updateProcess()
{
    model->removeRows(1, model->rowCount() - 1);

    QStringList procList = QDir("/proc").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &pid : procList) {
        QString exePath = QString("/proc/%1/exe").arg(pid);
        if (QFile::exists(exePath)) {
            QFileInfo fileInfo(exePath);
            QString path = fileInfo.symLinkTarget();
            if (path.isEmpty()) {
                path = exePath;
            }

            model->appendRow({new QStandardItem(pid), new QStandardItem(path)});
        }
    }
}
