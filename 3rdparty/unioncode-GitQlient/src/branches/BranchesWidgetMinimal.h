#pragma once

#include <QFrame>

class GitCache;
class GitBase;
class QPushButton;
class QToolButton;
class QMenu;

class BranchesWidgetMinimal : public QFrame
{
   Q_OBJECT
signals:
   void showFullBranchesView();
   void commitSelected(const QString &sha);
   void stashSelected(const QString &stashId);

public:
   explicit BranchesWidgetMinimal(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> git,
                                  QWidget *parent = nullptr);

   void configureLocalMenu(const QString &sha, const QString &branch);
   void configureRemoteMenu(const QString &sha, const QString &branch);
   void configureTagsMenu(const QString &sha, const QString &tag);
   void configureStashesMenu(const QString &stashId, const QString &name);
   void configureSubmodulesMenu(const QString &name);

   void clearActions();

private:
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitCache> mCache;
   QPushButton *mBack = nullptr;
   QToolButton *mLocal = nullptr;
   QMenu *mLocalMenu = nullptr;
   QToolButton *mRemote = nullptr;
   QMenu *mRemoteMenu = nullptr;
   QToolButton *mTags = nullptr;
   QMenu *mTagsMenu = nullptr;
   QToolButton *mStashes = nullptr;
   QMenu *mStashesMenu = nullptr;
   QToolButton *mSubmodules = nullptr;
   QMenu *mSubmodulesMenu = nullptr;
   QMenu *mCurrentMenuShown = nullptr;

   bool eventFilter(QObject *obj, QEvent *event);
   void addActionToMenu(const QString &sha, const QString &name, QMenu *menu);
};
