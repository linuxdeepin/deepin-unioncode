// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "interpreterwidget.h"
#include "utils/utils.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <DComboBox>
#include <DTableView>
#include <DHeaderView>
#include <DLabel>
#include <DWidget>
#include <DFrame>
#include <DPushButton>
#include <DFileDialog>
#include <DDialog>
#include <DLineEdit>

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
    : QAbstractTableModel(parent), d(new InterpreterModelPrivate())
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

void InterpreterModel::setCustomData(QVector<QPair<QString, QString>> &data)
{
    beginResetModel();
    d->packageVector = data;
    endResetModel();
}

class InterpreterWidgetPrivate
{
    friend class InterpreterWidget;

    DComboBox *interpreterComboBox = nullptr;
    DLineEdit *pipSourceEdit = nullptr;

    //todo: modified it later: creating a generic component to manage the toolchain
    DPushButton *selectCustomInterpreter = nullptr;
    DPushButton *removeCustomInterpreter = nullptr;

    InterpreterConfig currentInterpreter;
    QList<ToolChainData::ToolChainParam> customInterpreters {};

    InterpreterModel *model = nullptr;
    QSharedPointer<ToolChainData> toolChainData;
    QMutex mutex;
};

InterpreterWidget::InterpreterWidget(QWidget *parent)
    : PageWidget(parent), d(new InterpreterWidgetPrivate())
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
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setColumnStretch(1, 1);

    DLabel *label = new DLabel(tr("Python Interpreter:"));
    label->setFixedWidth(180);
    d->interpreterComboBox = new DComboBox();
    d->interpreterComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QObject::connect(d->interpreterComboBox, &DComboBox::currentTextChanged,
                     this, &InterpreterWidget::updatePackageData);

    d->selectCustomInterpreter = new DPushButton(tr("Browse"), this);
    d->removeCustomInterpreter = new DPushButton(tr("Remove"), this);

    mainLayout->addWidget(label, 0, 0);
    mainLayout->addWidget(d->interpreterComboBox, 0, 1);
    mainLayout->addWidget(d->selectCustomInterpreter, 0, 2);
    mainLayout->addWidget(d->removeCustomInterpreter, 0, 3);

    auto tableframe = new DFrame(this);
    auto tablelayout = new QVBoxLayout(tableframe);
    tableframe->setLayout(tablelayout);

    DTableView *tableView = new DTableView();
    tableView->setFrameShape(QFrame::NoFrame);
    tableView->setAlternatingRowColors(true);
    tableView->setShowGrid(false);
    tableView->horizontalHeader()->setSectionResizeMode(DHeaderView::Stretch);
    tableView->verticalHeader()->hide();
    tablelayout->addWidget(tableView);

    DHeaderView *headerView = tableView->horizontalHeader();
    headerView->setDefaultAlignment(Qt::AlignLeft);

    tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    d->model = new InterpreterModel();
    tableView->setModel(d->model);
    tableView->setFixedHeight(180);
    mainLayout->addWidget(tableframe, 1, 0, 1, 4);

    d->pipSourceEdit = new DLineEdit(this);
    mainLayout->addWidget(new DLabel(tr("PIP Source:"), this), 2, 0);
    mainLayout->addWidget(d->pipSourceEdit, 2, 1, 1, 3);

    connect(d->selectCustomInterpreter, &QPushButton::clicked, this, [=]() {
        auto path = DFileDialog::getOpenFileName(this, tr("Select Local Interpreter"), QDir::homePath(), "Python interpreters (python* python3*)");
        if (path.isEmpty())
            return;
        auto name = QFileInfo(path).fileName();
        ToolChainData::ToolChainParam param { name, path };
        int index = 0;
        while (index < d->interpreterComboBox->count()) {
            auto temp = qvariant_cast<ToolChainData::ToolChainParam>(d->interpreterComboBox->itemData(index, Qt::UserRole + 1));
            if (param == temp) {
                DDialog dialog;
                dialog.setMessage(tr("Selected Interpreter already exists"));
                dialog.setWindowTitle(tr("Warning"));
                dialog.setIcon(QIcon::fromTheme("dialog-warning"));
                dialog.addButton(tr("Yes"));
                dialog.exec();
                return;
            }
            index++;
        }
        d->interpreterComboBox->insertItem(index, name + "(" + path + ")");
        d->interpreterComboBox->setItemData(index, QVariant::fromValue(param), Qt::UserRole + 1);
        d->interpreterComboBox->setCurrentIndex(index);
        d->customInterpreters.append(param);
    });
    connect(d->removeCustomInterpreter, &QPushButton::clicked, this, [=]() {
        DDialog dialog;
        dialog.setMessage(tr("Confirm the removal of this interpreter?"));
        dialog.setWindowTitle(tr("Remove"));
        dialog.setIcon(QIcon::fromTheme("dialog-warning"));
        dialog.insertButton(0, tr("Yes"));
        dialog.insertButton(1, tr("Cancel"));
        int code = dialog.exec();
        if (code == 0) {
            auto param = qvariant_cast<ToolChainData::ToolChainParam>(d->interpreterComboBox->currentData(Qt::UserRole + 1));
            if (!d->customInterpreters.contains(param)) {
                DDialog dialog;
                dialog.setMessage(tr("Default Interpreter can`t be removed"));
                dialog.setWindowTitle(tr("Warning"));
                dialog.setIcon(QIcon::fromTheme("dialog-warning"));
                dialog.addButton(tr("Yes"));
                dialog.exec();
                return;
            }
            d->customInterpreters.removeOne(param);
            d->interpreterComboBox->removeItem(d->interpreterComboBox->currentIndex());
        }
    });
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

void InterpreterWidget::updatePackageData()
{
    auto param = qvariant_cast<ToolChainData::ToolChainParam>(d->interpreterComboBox->currentData(Qt::UserRole + 1));
    QString cmd = param.path + " -m pip list";
    
    QtConcurrent::run(this, &InterpreterWidget::findPackages, cmd);
}

void InterpreterWidget::findPackages(const QString &cmd)
{
    QProcess process;
    connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            &process, [&](int exitcode, QProcess::ExitStatus exitStatus) {
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
                    QMutexLocker lk(&d->mutex);
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
    int index = d->interpreterComboBox->currentIndex();
    if (index < 0) {
        d->currentInterpreter.version = ToolChainData::ToolChainParam();
    } else {
        d->currentInterpreter.version = qvariant_cast<ToolChainData::ToolChainParam>(d->interpreterComboBox->itemData(index, Qt::UserRole + 1));
    }

    dataToMap(map);

    return true;
}

void InterpreterWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    d->customInterpreters.clear();
    mapToData(map);

    d->interpreterComboBox->clear();
    updateUi();
    int count = d->interpreterComboBox->count();
    //append custom interpreters
    for (auto interpreter : d->customInterpreters) {
        d->interpreterComboBox->insertItem(count, interpreter.name + "(" + interpreter.path + ")");
        d->interpreterComboBox->setItemData(count, QVariant::fromValue(interpreter), Qt::UserRole + 1);
        count++;
    }

    for (int i = 0; i < count; i++) {
        ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(d->interpreterComboBox->itemData(i, Qt::UserRole + 1));
        if (d->currentInterpreter.version == toolChainParam) {
            d->interpreterComboBox->setCurrentIndex(i);
            break;
        }
    }
}

bool InterpreterWidget::dataToMap(QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> currentInterpreter;
    currentInterpreter.insert("name", d->currentInterpreter.version.name);
    currentInterpreter.insert("path", d->currentInterpreter.version.path);

    QVariantList list;
    for (auto interpreter : d->customInterpreters) {
        QMap<QString, QVariant> temp;
        temp.insert("name", interpreter.name);
        temp.insert("path", interpreter.path);
        list.append(temp);
    }

    map.insert("version", currentInterpreter);
    map.insert("customInterpreters", list);
    map.insert("pipSource", d->pipSourceEdit->text());
    return true;
}

bool InterpreterWidget::mapToData(const QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> currentInterpreter = map.value("version").toMap();
    d->currentInterpreter.version.name = currentInterpreter.value("name").toString();
    d->currentInterpreter.version.path = currentInterpreter.value("path").toString();

    auto variantList = map.value("customInterpreters").toList();
    for (QVariant variant : variantList) {
        auto map = variant.toMap();
        ToolChainData::ToolChainParam interpreter { map.value("name").toString(), map.value("path").toString() };
        d->customInterpreters.append(interpreter);
    }

    if (map.contains("pipSource")) {
        const auto &pipSrc = map.value("pipSource").toString();
        d->pipSourceEdit->setText(pipSrc);
    } else {
        d->pipSourceEdit->setText(Utils::defaultPIPSource());
    }

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
