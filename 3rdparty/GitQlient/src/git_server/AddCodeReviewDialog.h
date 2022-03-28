#pragma once

#include <QDialog>

namespace Ui
{
class AddCodeReviewDialog;
}

enum class ReviewMode
{
   Approve,
   RequestChanges,
   Comment
};

class AddCodeReviewDialog : public QDialog
{
   Q_OBJECT

signals:
   void commentAdded(const QString &commentText);

public:
   explicit AddCodeReviewDialog(ReviewMode mode, QWidget *parent = nullptr);
   ~AddCodeReviewDialog();

   void accept() override;

   QString getText() const;

private:
   Ui::AddCodeReviewDialog *ui;
   ReviewMode mMode;
};
