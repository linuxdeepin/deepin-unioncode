#include "ConflictButton.h"

#include <GitBase.h>
#include <GitLocal.h>
#include <QLogger.h>

#include <QHBoxLayout>
#include <QProcess>
#include <QPushButton>

using namespace QLogger;

ConflictButton::ConflictButton(const QString &filename, bool inConflict, const QSharedPointer<GitBase> &git,
                               QWidget *parent)
   : QFrame(parent)
   , mGit(git)
   , mFileName(filename)
   , mFile(new QPushButton(mFileName))
   , mEdit(new QPushButton())
   , mResolve(new QPushButton())
   , mUpdate(new QPushButton())
{
   mFile->setCheckable(true);
   mFile->setChecked(inConflict);

   mEdit->setIcon(QIcon(":/icons/edit"));
   mEdit->setFixedSize(30, 30);
   mResolve->setIcon(QIcon(":/icons/check"));
   mResolve->setFixedSize(30, 30);
   mUpdate->setIcon(QIcon(":/icons/refresh"));
   mUpdate->setFixedSize(30, 30);

   const auto layout = new QHBoxLayout(this);
   layout->setSpacing(0);
   layout->setContentsMargins(QMargins());
   layout->addWidget(mFile);
   layout->addWidget(mEdit);
   layout->addWidget(mUpdate);
   layout->addWidget(mResolve);

   mUpdate->setVisible(inConflict);
   mResolve->setVisible(inConflict);

   connect(mFile, &QPushButton::clicked, this, &ConflictButton::clicked);
   connect(mEdit, &QPushButton::clicked, this, [this] { emit signalEditFile(getFileName(), 0, 0); });
   connect(mResolve, &QPushButton::clicked, this, &ConflictButton::resolveConflict);
   connect(mUpdate, &QPushButton::clicked, this, [this]() { emit updateRequested(); });
}

void ConflictButton::setChecked(bool checked)
{
   mFile->setChecked(checked);
}

QString ConflictButton::getFileName() const
{
   return mGit->getWorkingDir() + "/" + mFileName;
}

void ConflictButton::setInConflict(bool inConflict)
{
   mUpdate->setVisible(inConflict);
   mResolve->setVisible(inConflict);
}

void ConflictButton::resolveConflict()
{
   QScopedPointer<GitLocal> git(new GitLocal(mGit));
   const auto ret = git->stageFile(mFileName);

   if (ret.success)
   {
      setInConflict(false);
      emit resolved();
   }
}
