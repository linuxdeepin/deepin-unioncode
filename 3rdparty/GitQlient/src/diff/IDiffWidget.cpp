#include "IDiffWidget.h"

IDiffWidget::IDiffWidget(const QSharedPointer<GitBase> &git, QSharedPointer<GitCache> cache, QWidget *parent)
   : QFrame(parent)
   , mGit(git)
   , mCache(cache)
{
}
