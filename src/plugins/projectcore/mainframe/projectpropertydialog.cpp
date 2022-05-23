#include "projectpropertydialog.h"

ProjectPropertyDialog::ProjectPropertyDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog (parent)
    , vLayout (new QVBoxLayout)
    , textBrowser (new QTextBrowser)
{
    setWindowTitle(QDialog::tr("Project Property"));
    vLayout->addWidget(textBrowser);
    setLayout(vLayout);
    setMinimumSize(450, 600);
}

void ProjectPropertyDialog::setPropertyText(const QString &text)
{
    textBrowser->setText(text);
}
