// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadcoredialog.h"
#include "event_man.h"

#include <DComboBox>
#include <DLineEdit>
#include <DPushButton>
#include <DFileDialog>
#include <DSuggestButton>
#include <DStyle>
#include <DIconTheme>

#include <QFormLayout>
#include <QDir>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
namespace ReverseDebugger {
namespace Internal {

class StartCoredumpDialogPrivate
{
public:
    DLineEdit *traceDir = nullptr;
    DComboBox *pidInput = nullptr;
    DComboBox *historyComboBox = nullptr;
};

LoadCoreDialog::LoadCoreDialog(QWidget *parent)
    : DDialog(parent),
      d(new StartCoredumpDialogPrivate)
{
    setWindowTitle(tr("Event Debugger Configure"));
    setIcon(DIconTheme::findQIcon("ide"));

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
    QFrame *mainFrame = new QFrame(this);
    addContent(mainFrame);
    auto centerLayout = new QVBoxLayout(mainFrame);
    mainFrame->setLayout(centerLayout);

    // trace directory.
    d->traceDir = new DLineEdit(this);
    d->traceDir->setPlaceholderText(tr("Trace directory."));

    DSuggestButton *btnBrowser = new DSuggestButton(this);
    btnBrowser->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));
    btnBrowser->setIconSize(QSize(24, 24));
    btnBrowser->setFixedSize(36, 36);


    // pid
    d->pidInput = new DComboBox(mainFrame);

    // history
    d->historyComboBox = new DComboBox(mainFrame);

    // ok & cancel button.
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("OK", "button"));
    addButton(buttonTexts[0], false);
    addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    setDefaultButton(1);

    auto hLayout = new QHBoxLayout(mainFrame);
    hLayout->addWidget(d->traceDir);
    hLayout->addWidget(btnBrowser);

    auto formLayout = new QFormLayout(mainFrame);
    formLayout->addRow(tr("trace directory："), hLayout);
    formLayout->addRow(tr("process ID："), d->pidInput);
    formLayout->addRow(tr("recent："), d->historyComboBox);
    formLayout->setSpacing(10);
    formLayout->setMargin(0);
    formLayout->setContentsMargins(10, 0, 0, 0);

    centerLayout->addLayout(formLayout);
    centerLayout->addStretch();
    centerLayout->setMargin(0);
    centerLayout->setContentsMargins(0, 0, 0, 5);

    connect(d->traceDir, &DLineEdit::editingFinished,
            this, &LoadCoreDialog::updatePid);

    connect(btnBrowser, &DSuggestButton::clicked, this, &LoadCoreDialog::showFileDialog);

    connect(d->historyComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::currentIndexChanged),
            this, &LoadCoreDialog::historyIndexChanged);

    updatePid();
}

void LoadCoreDialog::updatePid()
{
    QString traceDir = d->traceDir->text();
    QDir dir(traceDir);
    bool okEnabled = dir.exists();
    getButton(1)->setEnabled(okEnabled && !traceDir.isEmpty());

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
    } else {
        d->traceDir->showAlertMessage(tr("Invalid Path!"), 500);

    }
}

void LoadCoreDialog::historyIndexChanged(int)
{
    // do something.
}

void LoadCoreDialog::showFileDialog()
{
    QString dir = DFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    d->traceDir->text(),
                                                    DFileDialog::ShowDirsOnly
                                                            | DFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
        d->traceDir->setText(dir);
}

void LoadCoreDialog::onButtonClicked(const int &index)
{
    if (index == 1)
        DDialog::accept();
    else
        DDialog::reject();
}


}   // namespace ReverseDebugger
}   // namespace Internal
