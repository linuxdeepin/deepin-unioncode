#include "ProgressDlg.h"

#include <GitQlientStyles.h>

#include <QKeyEvent>

ProgressDlg::ProgressDlg(const QString &labelText, const QString &cancelButtonText, int maximum, bool autoClose)
   : QProgressDialog(labelText, cancelButtonText, 0, maximum)
{
   setAutoClose(autoClose);
   setAutoReset(false);
   setAttribute(Qt::WA_DeleteOnClose);
   setWindowModality(Qt::ApplicationModal);
   setWindowFlags(Qt::FramelessWindowHint);

   setStyleSheet(GitQlientStyles::getStyles());
}

void ProgressDlg::keyPressEvent(QKeyEvent *e)
{
   const auto key = e->key();

   if (key == Qt::Key_Escape)
      return;

   QProgressDialog::keyPressEvent(e);
}

void ProgressDlg::closeEvent(QCloseEvent *e)
{
   if (!mPrepareToClose)
      e->ignore();
   else
      QProgressDialog::closeEvent(e);
}

void ProgressDlg::close()
{
   mPrepareToClose = true;

   QProgressDialog::close();
}
