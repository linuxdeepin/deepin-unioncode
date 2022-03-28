#pragma once

#include <QDialog>

namespace Ui
{
class SquashDlg;
}

class GitCache;
class GitBase;

class SquashDlg : public QDialog
{
   Q_OBJECT

signals:
   void changesCommitted();

public:
   explicit SquashDlg(const QSharedPointer<GitBase> git, const QSharedPointer<GitCache> &cache, const QStringList &shas,
                      QWidget *parent = nullptr);
   ~SquashDlg() override;

   void accept() override;

private:
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitCache> mCache;
   QStringList mShas;
   Ui::SquashDlg *ui;
   int mTitleMaxLength = 50;

   void updateCounter(const QString &text);
   bool checkMsg(QString &msg);
};
