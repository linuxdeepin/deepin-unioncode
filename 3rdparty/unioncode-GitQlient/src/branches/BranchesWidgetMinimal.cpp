#include "BranchesWidgetMinimal.h"

#include <GitCache.h>
#include <GitSubmodules.h>
#include <GitStashes.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <QPushButton>
#include <QEvent>

BranchesWidgetMinimal::BranchesWidgetMinimal(const QSharedPointer<GitCache> &cache,
                                             const QSharedPointer<GitBase> git, QWidget *parent)
   : QFrame(parent)
   , mGit(git)
   , mCache(cache)
   , mBack(new QPushButton())
   , mLocal(new QToolButton())
   , mLocalMenu(new QMenu(mLocal))
   , mRemote(new QToolButton())
   , mRemoteMenu(new QMenu(mRemote))
   , mTags(new QToolButton())
   , mTagsMenu(new QMenu(mTags))
   , mStashes(new QToolButton())
   , mStashesMenu(new QMenu(mStashes))
   , mSubmodules(new QToolButton())
   , mSubmodulesMenu(new QMenu(mSubmodules))
{
   mBack->setIcon(QIcon(":/icons/back"));
   mBack->setToolTip(tr("Full view"));
   connect(mBack, &QPushButton::clicked, this, &BranchesWidgetMinimal::showFullBranchesView);

   const auto layout = new QVBoxLayout(this);
   layout->setContentsMargins(QMargins());
   layout->setSpacing(0);
   layout->addWidget(mBack);
   layout->addWidget(mLocal);
   layout->addWidget(mRemote);
   layout->addWidget(mTags);
   layout->addWidget(mStashes);
   layout->addWidget(mSubmodules);

   mLocalMenu->installEventFilter(this);
   mLocal->setMenu(mLocalMenu);
   mLocal->setIcon(QIcon(":/icons/local"));
   mLocal->setPopupMode(QToolButton::InstantPopup);
   mLocal->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   mLocal->setText("   " + QString::number(mLocalMenu->actions().count()));
   mLocal->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   mLocal->setToolTip(tr("Local branches"));

   mRemoteMenu->installEventFilter(this);
   mRemote->setMenu(mRemoteMenu);
   mRemote->setIcon(QIcon(":/icons/server"));
   mRemote->setPopupMode(QToolButton::InstantPopup);
   mRemote->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   mRemote->setText("   " + QString::number(mRemoteMenu->actions().count()));
   mRemote->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   mRemote->setToolTip(tr("Remote branches"));

   mTagsMenu->installEventFilter(this);
   mTags->setMenu(mTagsMenu);
   mTags->setIcon(QIcon(":/icons/tags"));
   mTags->setPopupMode(QToolButton::InstantPopup);
   mTags->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   mTags->setText("   " + QString::number(mTagsMenu->actions().count()));
   mTags->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   mTags->setToolTip(tr("Tags"));

   mStashesMenu->installEventFilter(this);
   mStashes->setMenu(mStashesMenu);
   mStashes->setIcon(QIcon(":/icons/stashes"));
   mStashes->setPopupMode(QToolButton::InstantPopup);
   mStashes->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   mStashes->setText("   " + QString::number(mStashesMenu->actions().count()));
   mStashes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   mStashes->setToolTip(tr("Stashes"));

   mSubmodulesMenu->installEventFilter(this);
   mSubmodules->setMenu(mSubmodulesMenu);
   mSubmodules->setIcon(QIcon(":/icons/submodules"));
   mSubmodules->setPopupMode(QToolButton::InstantPopup);
   mSubmodules->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   mSubmodules->setText("   " + QString::number(mSubmodulesMenu->actions().count()));
   mSubmodules->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   mSubmodules->setToolTip(tr("Submodules"));
}

bool BranchesWidgetMinimal::eventFilter(QObject *obj, QEvent *event)
{

   if (const auto menu = qobject_cast<QMenu *>(obj); menu && event->type() == QEvent::Show)
   {
      auto localPos = menu->parentWidget()->pos();
      localPos.setX(localPos.x());
      auto pos = mapToGlobal(localPos);
      menu->show();
      pos.setX(pos.x() - menu->width());
      menu->move(pos);
      return true;
   }

   return false;
}

void BranchesWidgetMinimal::addActionToMenu(const QString &sha, const QString &name, QMenu *menu)
{
   const auto action = new QAction(name, menu);
   action->setData(sha);
   connect(action, &QAction::triggered, this, [this, sha] { emit commitSelected(sha); });

   menu->addAction(action);
}

void BranchesWidgetMinimal::configureLocalMenu(const QString &sha, const QString &branch)
{
   addActionToMenu(sha, branch, mLocalMenu);
   mLocal->setText("   " + QString::number(mLocalMenu->actions().count()));
}

void BranchesWidgetMinimal::configureRemoteMenu(const QString &sha, const QString &branch)
{
   addActionToMenu(sha, branch, mRemoteMenu);
   mRemote->setText("   " + QString::number(mRemoteMenu->actions().count()));
}

void BranchesWidgetMinimal::configureTagsMenu(const QString &sha, const QString &tag)
{
   addActionToMenu(sha, tag, mTagsMenu);
   mTags->setText("   " + QString::number(mTagsMenu->actions().count()));
}

void BranchesWidgetMinimal::configureStashesMenu(const QString &stashId, const QString &name)
{
   const auto action = new QAction(name);
   action->setData(stashId);
   connect(action, &QAction::triggered, this, [this, stashId] { emit stashSelected(stashId); });

   mStashesMenu->addAction(action);
   mStashes->setText("   " + QString::number(mStashesMenu->actions().count()));
}

void BranchesWidgetMinimal::configureSubmodulesMenu(const QString &name)
{
   const auto action = new QAction(name);
   action->setData(name);
   mSubmodulesMenu->addAction(action);
   mSubmodules->setText("   " + QString::number(mSubmodulesMenu->actions().count()));
}

void BranchesWidgetMinimal::clearActions()
{
   mLocalMenu->clear();
   mRemoteMenu->clear();
   mTagsMenu->clear();
   mStashesMenu->clear();
   mSubmodulesMenu->clear();
}
