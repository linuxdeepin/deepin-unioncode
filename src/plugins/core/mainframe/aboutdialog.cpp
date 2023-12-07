// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aboutdialog.h"
#include "common/common.h"

#include <DLabel>
#include <DPushButton>
#include <DTextEdit>
#include <DMessageBox>
#include <DTitlebar>

#include <QGridLayout>
#include <QLatin1String>
#include <QDateTime>
#include <QDesktopServices>

DWIDGET_USE_NAMESPACE

AboutDialog::AboutDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    setupUi();
}

AboutDialog::~AboutDialog()
{

}

void AboutDialog::setupUi()
{
    DTitlebar *titleBar = new DTitlebar();
    titleBar->setMenuVisible(false);
    titleBar->setTitle(QString(tr("About Deepin Union Code")));

    DLabel *logoLabel = new DLabel(this);
    logoLabel->setPixmap(QPixmap(QIcon::fromTheme("ide").pixmap(32, 32)));

    QString buildDateInfo = tr("<br/>Built on %1 %2 in %3<br/>")
            .arg(QLatin1String(__DATE__), QLatin1String(__TIME__), ProcessUtil::localPlatform());

    const QString description = tr(
        "<h3>Deepin Union Code %1</h3>"
        "%2<br/>"
        "Copyright 2019-%3 UnionTech Software Technology Co., Ltd. All rights reserved.<br/><br/>"
        "This program is released under <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GPL-3.0-or-later</a>; \
               we hope that the scheme will be useful, \
               but we do not guarantee that it will be of economic value or fit for a particular purpose. \
               For more information, see the GNU General Public License. <br/>\
               Thanks to all the <a href=\"opensourcesoftware\">open source software</a> used."
        "<br/>")
        .arg(version(),
             buildDateInfo,
             QString::number(QDateTime::currentDateTime().date().year()));

    DLabel *copyRightLabel = new DLabel(description);
    copyRightLabel->setWordWrap(true);
    copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    connect(copyRightLabel, &DLabel::linkActivated, this, &AboutDialog::handleLinkActivated);

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    DPushButton *closeButton = new DPushButton(DPushButton::tr("Close"));
    connect(closeButton , &DPushButton::clicked, this, &DAbstractDialog::reject);
    layoutButtons->addStretch();
    layoutButtons->addWidget(closeButton);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    auto detailLayout = new QGridLayout(this);
    detailLayout->setSizeConstraint(QLayout::SetFixedSize);
    detailLayout->addWidget(logoLabel , 1, 0, 1, 1);
    detailLayout->addWidget(copyRightLabel, 0, 4, 4, 5);
    detailLayout->addLayout(layoutButtons, 4, 4, 1, 5);
    vLayout->addWidget(titleBar);
    vLayout->addLayout(detailLayout);
}

void AboutDialog::handleLinkActivated(const QString& link)
{
    if (link == "opensourcesoftware" && !bExpand) {
        QFile file("/usr/share/doc/deepin-unioncode/copyright");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            DTextEdit* textEdit = new DTextEdit(this);
            layout()->addWidget(textEdit);

            QTextStream stream(&file);
            QString fileContent = stream.readAll();
            textEdit->setPlainText(fileContent);
            setFixedHeight(height() * 2);
            bExpand = true;
        } else {
            DMessageBox::critical(this, tr("Error"), tr("Failed to open the file."));
        }
    } else {
        QDesktopServices::openUrl(QUrl(link));
    }
}
