#include "processdialog.h"
#include "common/common.h"
#include <QLabel>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QVBoxLayout>

class ProcessDialogPrivate
{
    friend class ProcessDialog;
    StatusWidget *statusWidget = nullptr;
    QTextBrowser *messageBrowser = nullptr;
    QHBoxLayout *hLayout = nullptr;
};

ProcessDialog::ProcessDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog (parent, f)
    , d (new ProcessDialogPrivate)
{
    setAutoFillBackground(true);
    int lineHeight = this->fontMetrics().height();
    this->setMinimumWidth(500);
    this->setMinimumHeight(lineHeight * 2);
    d->statusWidget = new StatusWidget();
    d->statusWidget->setPatternFlags(StatusWidget::PatternFlag::Ring);
    d->statusWidget->setFixedSize(lineHeight * 2, lineHeight * 2);
    d->messageBrowser = new QTextBrowser();
    d->hLayout = new QHBoxLayout();

    this->setLayout(d->hLayout);
    d->hLayout->addWidget(d->statusWidget);
    d->hLayout->addWidget(d->messageBrowser);
}

ProcessDialog::~ProcessDialog()
{
    if (d) {
        delete d;
    }
}

ProcessDialog *ProcessDialog::globalInstance()
{
    static ProcessDialog dialog;
    return &dialog;
}

void ProcessDialog::setRunning(bool runable)
{
    if (runable)
        d->statusWidget->start();
    else
        d->statusWidget->stop();
}

ProcessDialog &ProcessDialog::operator <<(const QString &message)
{
    d->messageBrowser->append(message);
    return *this;
}

void ProcessDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
}
