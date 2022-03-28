#include "AddCodeReviewDialog.h"
#include "ui_AddCodeReviewDialog.h"

#include <GitQlientStyles.h>

#include <QMessageBox>

AddCodeReviewDialog::AddCodeReviewDialog(ReviewMode mode, QWidget *parent)
   : QDialog(parent)
   , ui(new Ui::AddCodeReviewDialog)
   , mMode(mode)
{
   ui->setupUi(this);

   if (!parent)
      setStyleSheet(GitQlientStyles::getInstance()->getStyles());

   switch (mode)
   {
      case ReviewMode::Comment:
         setWindowTitle(tr("Add comment"));
         break;
      case ReviewMode::Approve:
         setWindowTitle(tr("Approve PR"));
         break;
      case ReviewMode::RequestChanges:
         setWindowTitle(tr("Request changes"));
      default:
         break;
   }

   setAttribute(Qt::WA_DeleteOnClose);
}

AddCodeReviewDialog::~AddCodeReviewDialog()
{
   delete ui;
}

void AddCodeReviewDialog::accept()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
   if (const auto text = ui->teComment->toMarkdown(); !text.isEmpty())
#else
   if (const auto text = ui->teComment->toPlainText(); !text.isEmpty())
#endif
   {
      emit commentAdded(text);
      QDialog::accept();
   }
   else if (mMode != ReviewMode::Approve)
      QMessageBox::warning(this, tr("Empty comment!"),
                           tr("The body cannot be empty when adding a comment or requesting changes."));
}

QString AddCodeReviewDialog::getText() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
   return ui->teComment->toMarkdown();
#else
   return ui->teComment->toPlainText();
#endif
}
