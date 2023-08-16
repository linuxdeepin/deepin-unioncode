// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadcoredialog.h"
#include "event_man.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QDir>
#include <QFileDialog>

namespace ReverseDebugger {
namespace Internal {

class StartCoredumpDialogPrivate
{
public:
    QLineEdit *traceDir = nullptr;
    QComboBox *pidInput = nullptr;
    QComboBox *historyComboBox = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
};

LoadCoreDialog::LoadCoreDialog(QWidget *parent)
    : QDialog(parent),
      d(new StartCoredumpDialogPrivate)
{
    setupUi();
}

LoadCoreDialog::~LoadCoreDialog()
{
}

CoredumpRunParameters LoadCoreDialog::displayDlg(const QString &traceDir)
{
    d->traceDir->setText(traceDir);

    CoredumpRunParameters ret;
    auto code = exec();
    if (code == QDialog::Accepted) {
        ret.pid = d->pidInput->currentText().toInt();
        ret.tracedir = d->traceDir->text();
    }

    return ret;
}

void LoadCoreDialog::setupUi()
{
    setWindowTitle(tr("Event Debugger Configure"));

    // trace directory.
    d->traceDir = new QLineEdit(this);
    d->traceDir->setPlaceholderText(tr("Trace directory."));

    QPushButton *btnBrowser = new QPushButton(this);
    btnBrowser->setText(tr("Browse..."));

    // pid
    d->pidInput = new QComboBox(this);

    // history
    d->historyComboBox = new QComboBox(this);

    // ok & cancel button.
    d->buttonBox = new QDialogButtonBox(this);
    d->buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    d->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    d->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    d->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    auto hLayout = new QHBoxLayout();
    hLayout->addWidget(d->traceDir);
    hLayout->addWidget(btnBrowser);

    auto splitLine = [this]() -> QFrame * {
            auto splitLine = new QFrame(this);
            splitLine->setFrameShape(QFrame::HLine);
            splitLine->setFrameShadow(QFrame::Sunken);
            return splitLine; };

    auto formLayout = new QFormLayout();
    formLayout->addRow(tr("trace directory："), hLayout);
    formLayout->addRow(tr("process ID："), d->pidInput);
    formLayout->addRow(splitLine());
    formLayout->addRow(tr("recent："), d->historyComboBox);

    auto centerLayout = new QVBoxLayout(this);
    centerLayout->addLayout(formLayout);
    centerLayout->addStretch();
    centerLayout->addWidget(splitLine());
    centerLayout->addWidget(d->buttonBox);

    connect(d->traceDir, &QLineEdit::textChanged,
            this, &LoadCoreDialog::updatePid);

    connect(btnBrowser, &QPushButton::clicked, this, &LoadCoreDialog::showFileDialog);

    connect(d->historyComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LoadCoreDialog::historyIndexChanged);

    connect(d->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(d->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updatePid();
}

void LoadCoreDialog::updatePid()
{
    QString traceDir = d->traceDir->text();
    QDir dir(traceDir);
    bool okEnabled = dir.exists();
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(okEnabled);

    // fill pid combo list here!
    if (okEnabled) {
        d->pidInput->clear();

        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        QString mapname = QLatin1String(MAP_FILE_NAME);
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            if (0 == fileInfo.fileName().indexOf(mapname)) {
                d->pidInput->addItem(fileInfo.fileName().mid(mapname.size()));
            }
        }

        d->pidInput->setCurrentIndex(0);
    }
}

void LoadCoreDialog::historyIndexChanged(int)
{
    // do something.
}

void LoadCoreDialog::showFileDialog()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    d->traceDir->text(),
                                                    QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
        d->traceDir->setText(dir);
}

}   // namespace ReverseDebugger
}   // namespace Internal
