// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "interperterwidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <DComboBox>
#include <DTableView>
#include <DHeaderView>
#include <DLabel>
#include <DWidget>

#include <QtConcurrent>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
class InterpreterModelPrivate
{
    QVector<QPair<QString, QString>> packageVector;

    friend class InterpreterModel;
};

InterpreterModel::InterpreterModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d(new InterpreterModelPrivate())
{

}

InterpreterModel::~InterpreterModel()
{

}

int InterpreterModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return d->packageVector.count();
}

int InterpreterModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return ColumnID::_KCount;
}

QVariant InterpreterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::ForegroundRole)
        return QVariant();

    if (index.row() >= d->packageVector.count())
        return QVariant();

    QString package = d->packageVector.at(index.row()).first;
    QString version = d->packageVector.at(index.row()).second;

    switch (index.column()) {
        case ColumnID::kPackage:
            return package;
        case ColumnID::kVersion:
            return version;
        default:
            return QVariant();
    }
}

QVariant InterpreterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case ColumnID::kPackage:
            return tr("Package");
        case ColumnID::kVersion:
            return tr("Version");
        default:
            return QVariant();
    }
}

void InterpreterModel::setCustomData(QVector<QPair<QString, QString>>& data)
{
    beginResetModel();
    d->packageVector = data;
    endResetModel();
}

class InterperterWidgetPrivate
{
    friend class InterpreterWidget;

    DComboBox *interpreterComboBox = nullptr;
    InterpreterModel *model = nullptr;
    QSharedPointer<ToolChainData> toolChainData;
};

InterpreterWidget::InterpreterWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new InterperterWidgetPrivate())
{
    d->toolChainData.reset(new ToolChainData());
    QString retMsg;
    bool ret = d->toolChainData->readToolChainData(retMsg);
    if (ret) {
        qInfo() << retMsg;
    }

    setupUi();
    updateUi();
}

InterpreterWidget::~InterpreterWidget()
{
    if (d) {
        delete d;
    }
}

void InterpreterWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    DWidget *interpreterWidget = new DWidget();
    vLayout->addWidget(interpreterWidget);

    QHBoxLayout *hLayout = new QHBoxLayout();
    interpreterWidget->setLayout(hLayout);
    DLabel *label = new DLabel(DLabel::tr("Python Interperter:"));
    label->setFixedWidth(180);
    d->interpreterComboBox = new DComboBox();
    QObject::connect(d->interpreterComboBox, &DComboBox::currentTextChanged,
                     this, &InterpreterWidget::setPackageData);

    hLayout->addWidget(label);
    hLayout->addWidget(d->interpreterComboBox);

    DTableView *tableView = new DTableView();
    tableView->setShowGrid(false);
    tableView->horizontalHeader()->setSectionResizeMode(DHeaderView::Stretch);
    tableView->verticalHeader()->hide();
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->model = new InterpreterModel();
    tableView->setModel(d->model);
    vLayout->addWidget(tableView);
}

void InterpreterWidget::updateUi()
{
    const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();
    ToolChainData::Params cParams = data.value(kPython);
    int i = 0;
    for (auto param : cParams) {
        QString text = param.name + "(" + param.path + ")";
        d->interpreterComboBox->insertItem(i, text);
        d->interpreterComboBox->setItemData(i, QVariant::fromValue(param), Qt::UserRole + 1);
        i++;
    }
}

void InterpreterWidget::setPackageData(const QString &text)
{
    QString cmd = "pip list";
    if (text.indexOf("python3") > -1) {
        cmd = "pip3 list";
    }
    QtConcurrent::run(this, &InterpreterWidget::findPackages, cmd);
}

void InterpreterWidget::findPackages(const QString &cmd)
{
    QProcess process;
    connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
        if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
            QString output = QString(process.readAllStandardOutput());

            QVector<QPair<QString, QString>> dataVector;
            QStringList list = output.split("\n");
            foreach (QString value, list) {
                value = value.trimmed();
                value = value.replace(QRegExp("[\\s]+"), ",");
                QStringList sublist = value.split(",");
                if (sublist.count() > 1) {
                    if (sublist.at(0).indexOf("Package") > -1
                            || sublist.at(0).indexOf("----") > -1) {
                        continue;
                    }
                    dataVector.append(QPair<QString, QString>(sublist.at(0).trimmed(), sublist.at(1).trimmed()));
                }
            }
            d->model->setCustomData(dataVector);
        } else {
            qInfo() << "Error" << exitcode << exitStatus;
        }
    });

    process.start(cmd);
    process.waitForFinished();
}

bool InterpreterWidget::getControlValue(QMap<QString, QVariant> &map)
{
    InterpreterConfig config;
    int index = d->interpreterComboBox->currentIndex();
    if (index < 0) {
        config.version = ToolChainData::ToolChainParam();
    } else {
        config.version = qvariant_cast<ToolChainData::ToolChainParam>(d->interpreterComboBox->itemData(index, Qt::UserRole + 1));
    }

    dataToMap(config, map);

    return true;
}

void InterpreterWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    InterpreterConfig config;
    mapToData(map, config);

    int count = d->interpreterComboBox->count();
    for (int i = 0; i < count; i++) {
        ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(d->interpreterComboBox->itemData(i, Qt::UserRole + 1));
        if (config.version.name == toolChainParam.name
                && config.version.path == toolChainParam.path) {
            d->interpreterComboBox->setCurrentIndex(i);
            break;
        }
    }
}

bool InterpreterWidget::dataToMap(const InterpreterConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> version;
    version.insert("name", config.version.name);
    version.insert("path", config.version.path);

    map.insert("version", version);

    return true;
}

bool InterpreterWidget::mapToData(const QMap<QString, QVariant> &map, InterpreterConfig &config)
{
    QMap<QString, QVariant> version = map.value("version").toMap();
    config.version.name = version.value("name").toString();
    config.version.path = version.value("path").toString();

    return true;
}

void InterpreterWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void InterpreterWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}

