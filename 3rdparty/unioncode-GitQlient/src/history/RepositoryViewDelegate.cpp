#include "RepositoryViewDelegate.h"

#include <Colors.h>
#include <CommitHistoryColumns.h>
#include <CommitHistoryModel.h>
#include <CommitHistoryView.h>
#include <CommitInfo.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitLocal.h>
#include <GitQlientStyles.h>
#include <GitServerCache.h>
#include <Lane.h>
#include <LaneType.h>
#include <PullRequest.h>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSortFilterProxyModel>
#include <QToolTip>
#include <QUrl>

using namespace GitServer;

static const int MIN_VIEW_WIDTH_PX = 480;

RepositoryViewDelegate::RepositoryViewDelegate(const QSharedPointer<GitCache> &cache,
                                               const QSharedPointer<GitBase> &git,
                                               const QSharedPointer<GitServerCache> &gitServerCache,
                                               CommitHistoryView *view)
   : mCache(cache)
   , mGit(git)
   , mGitServerCache(gitServerCache)
   , mView(view)
{
}

void RepositoryViewDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
   p->setRenderHints(QPainter::Antialiasing);

   QStyleOptionViewItem newOpt(opt);
   newOpt.font.setPointSize(9);

   if (newOpt.state & QStyle::State_Selected)
      p->fillRect(newOpt.rect, GitQlientStyles::getGraphSelectionColor());
   else if (newOpt.state & QStyle::State_MouseOver)
      p->fillRect(newOpt.rect, GitQlientStyles::getGraphHoverColor());

   const auto row = mView->hasActiveFilter()
       ? dynamic_cast<QSortFilterProxyModel *>(mView->model())->mapToSource(index).row()
       : index.row();

   const auto commit = mCache->commitInfo(row);

   if (commit.sha.isEmpty())
      return;

   if (index.column() == static_cast<int>(CommitHistoryColumns::Graph))
   {
      newOpt.rect.setX(newOpt.rect.x() + 10);
      paintGraph(p, newOpt, commit);
   }
   else if (index.column() == static_cast<int>(CommitHistoryColumns::Log))
      paintLog(p, newOpt, commit, index.data().toString());
   else
   {

      p->setPen(GitQlientStyles::getTextColor());
      newOpt.rect.setX(newOpt.rect.x() + 10);

      QTextOption textalignment(Qt::AlignLeft | Qt::AlignVCenter);
      auto text = index.data().toString();

      if (index.column() == static_cast<int>(CommitHistoryColumns::Date))
      {
         textalignment = QTextOption(Qt::AlignRight | Qt::AlignVCenter);
         const auto prev = QDateTime::fromString(mView->indexAbove(index).data().toString(), "dd MMM yyyy hh:mm");
         const auto current = QDateTime::fromString(text, "dd MMM yyyy hh:mm");

         if (current.date() == prev.date())
            text = current.toString("hh:mm");
         else
            text = current.toString("dd MMM yyyy - hh:mm");

         newOpt.rect.setWidth(newOpt.rect.width() - 5);
      }
      else if (index.column() == static_cast<int>(CommitHistoryColumns::Sha))
      {
         newOpt.font.setPointSize(8);
         newOpt.font.setFamily("DejaVu Sans Mono");

         text = commit.sha != CommitInfo::ZERO_SHA ? text.left(8) : "";
      }
      else if (index.column() == static_cast<int>(CommitHistoryColumns::Author) && commit.isSigned())
      {
         static const auto size = 15;
         static const auto offset = 5;
         QPixmap pic(QString::fromUtf8(commit.verifiedSignature() ? ":/icons/signed" : ":/icons/unsigned"));
         pic = pic.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

         const auto inc = (newOpt.rect.height() - size) / 2;

         p->drawPixmap(QRect(newOpt.rect.x(), newOpt.rect.y() + inc, size, size), pic);

         newOpt.rect.setX(newOpt.rect.x() + size + offset);
      }

      QFontMetrics fm(newOpt.font);
      p->setFont(newOpt.font);

      if (const auto cursorColumn = mView->indexAt(mView->mapFromGlobal(QCursor::pos())).column();
          newOpt.state & QStyle::State_MouseOver && cursorColumn == index.column()
          && cursorColumn == static_cast<int>(CommitHistoryColumns::Sha))
      {
         p->setPen(gitQlientOrange);
      }

      p->drawText(newOpt.rect, fm.elidedText(text, Qt::ElideRight, newOpt.rect.width()), textalignment);
   }
}

QSize RepositoryViewDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
   return QSize(LANE_WIDTH, ROW_HEIGHT);
}

bool RepositoryViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                         const QModelIndex &index)
{
   const auto cursorColumn = mView->indexAt(mView->mapFromGlobal(QCursor::pos())).column();

   if (event->type() == QEvent::MouseButtonPress && cursorColumn == index.column()
       && cursorColumn == static_cast<int>(CommitHistoryColumns::Sha))
   {
      mColumnPressed = cursorColumn;
      return true;
   }
   else if (event->type() == QEvent::MouseButtonRelease && cursorColumn == index.column() && mColumnPressed != -1)
   {
      const auto text = index.data().toString();
      if (cursorColumn == static_cast<int>(CommitHistoryColumns::Sha) && text != CommitInfo::ZERO_SHA)
      {
         QApplication::clipboard()->setText(text);
         QToolTip::showText(QCursor::pos(), tr("Copied!"), mView);
      }

      mColumnPressed = -1;
      return true;
   }

   return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void RepositoryViewDelegate::paintGraphLane(QPainter *p, const Lane &lane, bool laneHeadPresent, int x1, int x2,
                                            const QColor &col, const QColor &activeCol, const QColor &mergeColor,
                                            bool isWip, bool hasChilds) const
{
   const auto padding = 2;
   x1 += padding;
   x2 += padding;

   const auto h = ROW_HEIGHT / 2;
   const auto m = (x1 + x2) / 2;
   const auto r = (x2 - x1) * 1 / 3;
   const auto spanAngle = 90 * 16;
   const auto angleWidthRight = 2 * (x1 - m);
   const auto angleWidthLeft = 2 * (x2 - m);
   const auto angleHeightUp = 2 * h;
   const auto angleHeightDown = 2 * -h;

   static QPen lanePen(GitQlientStyles::getTextColor(), 2); // fast path here

   // arc
   lanePen.setBrush(col);
   p->setPen(lanePen);

   switch (lane.getType())
   {
      case LaneType::JOIN:
      case LaneType::JOIN_R:
      case LaneType::HEAD:
      case LaneType::HEAD_R: {
         p->drawArc(m, h, angleWidthRight, angleHeightUp, 0 * 16, spanAngle);
         break;
      }
      case LaneType::JOIN_L: {
         p->drawArc(m, h, angleWidthLeft, angleHeightUp, 90 * 16, spanAngle);
         break;
      }
      case LaneType::TAIL:
      case LaneType::TAIL_R: {
         p->drawArc(m, h, angleWidthRight, angleHeightDown, 270 * 16, spanAngle);
         break;
      }
      default:
         break;
   }

   if (isWip)
   {
      lanePen.setColor(activeCol);
      p->setPen(lanePen);
   }

   // vertical line
   if (!(isWip && !hasChilds))
   {
      if (!isWip && !hasChilds
          && (lane.getType() == LaneType::HEAD || lane.getType() == LaneType::INITIAL
              || lane.getType() == LaneType::BRANCH || lane.getType() == LaneType::MERGE_FORK
              || lane.getType() == LaneType::MERGE_FORK_R || lane.getType() == LaneType::MERGE_FORK_L
              || lane.getType() == LaneType::ACTIVE))
         p->drawLine(m, h, m, 2 * h);
      else
      {
         switch (lane.getType())
         {
            case LaneType::ACTIVE:
            case LaneType::NOT_ACTIVE:
            case LaneType::MERGE_FORK:
            case LaneType::MERGE_FORK_R:
            case LaneType::MERGE_FORK_L:
            case LaneType::JOIN:
            case LaneType::JOIN_R:
            case LaneType::JOIN_L:
            case LaneType::CROSS:
               p->drawLine(m, 0, m, 2 * h);
               break;
            case LaneType::HEAD_L:
            case LaneType::BRANCH:
               p->drawLine(m, h, m, 2 * h);
               break;
            case LaneType::TAIL_L:
            case LaneType::INITIAL:
               p->drawLine(m, 0, m, h);
               break;
            default:
               break;
         }
      }
   }

   // center symbol
   auto isCommit = false;

   if (isWip)
   {
      isCommit = true;
      p->setPen(QPen(col, 2));
      p->setBrush(col);
      p->drawEllipse(m - r + 2, h - r + 2, 8, 8);
   }
   else
   {
      switch (lane.getType())
      {
         case LaneType::HEAD:
         case LaneType::INITIAL:
         case LaneType::BRANCH:
         case LaneType::MERGE_FORK:
         case LaneType::MERGE_FORK_R:
            isCommit = true;
            p->setPen(QPen(mergeColor, 2));
            p->setBrush(col);
            p->drawEllipse(m - r + 2, h - r + 2, 8, 8);
            break;
         case LaneType::MERGE_FORK_L:
            isCommit = true;
            p->setPen(QPen(laneHeadPresent ? mergeColor : col, 2));
            p->setBrush(col);
            p->drawEllipse(m - r + 2, h - r + 2, 8, 8);
            break;
         case LaneType::ACTIVE: {
            isCommit = true;
            p->setPen(QPen(col, 2));
            p->setBrush(QColor(isWip ? col : GitQlientStyles::getBackgroundColor()));
            p->drawEllipse(m - r + 2, h - r + 2, 8, 8);
         }
         break;
         default:
            break;
      }
   }

   lanePen.setColor(mergeColor);
   p->setPen(lanePen);

   // horizontal line
   switch (lane.getType())
   {
      case LaneType::MERGE_FORK:
      case LaneType::JOIN:
      case LaneType::HEAD:
      case LaneType::TAIL:
      case LaneType::CROSS:
      case LaneType::CROSS_EMPTY:
         p->drawLine(x1 + (isCommit ? 10 : 0), h, x2, h);
         break;
      case LaneType::MERGE_FORK_R:
         p->drawLine(x1 + (isCommit ? 0 : 10), h, m - (isCommit ? 6 : 0), h);
         break;
      case LaneType::MERGE_FORK_L:
      case LaneType::HEAD_L:
      case LaneType::TAIL_L:
         p->drawLine(m + (isCommit ? 6 : 0), h, x2, h);
         break;
      default:
         break;
   }
}

QColor RepositoryViewDelegate::getMergeColor(const Lane &currentLane, const CommitInfo &commit, int currentLaneIndex,
                                             const QColor &defaultColor, bool &isSet) const
{
   auto mergeColor = defaultColor;
   //= GitQlientStyles::getBranchColorAt((commit.getLanesCount() - 1) % GitQlientStyles::getTotalBranchColors());

   switch (currentLane.getType())
   {
      case LaneType::HEAD_L:
      case LaneType::HEAD_R:
      case LaneType::TAIL_L:
      case LaneType::TAIL_R:
      case LaneType::MERGE_FORK_L:
      case LaneType::JOIN_R:
         isSet = true;
         mergeColor = defaultColor;
         break;
      case LaneType::MERGE_FORK_R:
      case LaneType::JOIN_L:
         for (auto laneCount = 0; laneCount < currentLaneIndex; ++laneCount)
         {
            if (commit.laneAt(laneCount).equals(LaneType::JOIN_L))
            {
               mergeColor = GitQlientStyles::getBranchColorAt(laneCount % GitQlientStyles::getTotalBranchColors());
               isSet = true;
               break;
            }
         }
         break;
      default:
         break;
   }

   return mergeColor;
}

void RepositoryViewDelegate::paintGraph(QPainter *p, const QStyleOptionViewItem &opt, const CommitInfo &commit) const
{
   p->save();
   p->setClipRect(opt.rect, Qt::IntersectClip);
   p->translate(opt.rect.topLeft());

   if (mView->hasActiveFilter())
   {
      const auto activeColor = GitQlientStyles::getBranchColorAt(0);
      paintGraphLane(p, LaneType::ACTIVE, false, 0, LANE_WIDTH, activeColor, activeColor, activeColor, false,
                     commit.hasChilds());
   }
   else
   {
      if (commit.sha == CommitInfo::ZERO_SHA)
      {
         const auto activeColor = GitQlientStyles::getBranchColorAt(0);
         QColor color = activeColor;

         if (mCache->pendingLocalChanges())
            color = gitQlientOrange;

         paintGraphLane(p, LaneType::BRANCH, false, 0, LANE_WIDTH, color, activeColor, activeColor, true,
                        commit.parentsCount() != 0);
      }
      else
      {
         const auto laneNum = commit.lanesCount();
         const auto activeLane = commit.getActiveLane();
         const auto activeColor
             = GitQlientStyles::getBranchColorAt(activeLane % GitQlientStyles::getTotalBranchColors());
         auto x1 = 0;
         auto isSet = false;
         auto laneHeadPresent = false;
         auto mergeColor = GitQlientStyles::getBranchColorAt((laneNum - 1) % GitQlientStyles::getTotalBranchColors());

         for (auto i = laneNum - 1, x2 = LANE_WIDTH * laneNum; i >= 0; --i, x2 -= LANE_WIDTH)
         {
            x1 = x2 - LANE_WIDTH;

            auto currentLane = commit.laneAt(i);

            if (!laneHeadPresent && i < laneNum - 1)
            {
               auto prevLane = commit.laneAt(i + 1);
               laneHeadPresent
                   = prevLane.isHead() || prevLane.equals(LaneType::JOIN_R) || prevLane.equals(LaneType::JOIN_L);
            }

            if (!currentLane.equals(LaneType::EMPTY))
            {
               auto color = activeColor;

               if (i != activeLane)
                  color = GitQlientStyles::getBranchColorAt(i % GitQlientStyles::getTotalBranchColors());

               if (!isSet)
                  mergeColor = getMergeColor(currentLane, commit, i, color, isSet);

               paintGraphLane(p, currentLane, laneHeadPresent, x1, x2, color, activeColor, mergeColor, false,
                              commit.hasChilds());

               if (mView->hasActiveFilter())
                  break;
            }
         }
      }
   }
   p->restore();
}

void RepositoryViewDelegate::paintLog(QPainter *p, const QStyleOptionViewItem &opt, const CommitInfo &commit,
                                      const QString &text) const
{
   const auto sha = commit.sha;

   if (sha.isEmpty())
      return;

   auto offset = 0;

   if (mGitServerCache)
   {
      if (const auto pr = mGitServerCache->getPullRequest(commit.sha); pr.isValid())
      {
         offset = 5;
         paintPrStatus(p, opt, offset, pr);
      }
   }

   paintTagBranch(p, opt, offset, sha);

   auto newOpt = opt;
   newOpt.rect.setX(opt.rect.x() + offset + 5);

   QFontMetrics fm(newOpt.font);

   p->setFont(newOpt.font);
   p->setPen(GitQlientStyles::getTextColor());
   p->drawText(newOpt.rect, fm.elidedText(text, Qt::ElideRight, newOpt.rect.width()),
               QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
}

void RepositoryViewDelegate::paintTagBranch(QPainter *painter, QStyleOptionViewItem o, int &startPoint,
                                            const QString &sha) const
{
   if (mCache->hasReferences(sha) && !mView->hasActiveFilter())
   {
      QVector<QString> marks;
      QVector<QColor> colors;
      const auto currentBranch = mGit->getCurrentBranch();

      if (startPoint == 0)
         startPoint = 5;

      if ((currentBranch.isEmpty() || currentBranch == "HEAD"))
      {
         if (const auto ret = mGit->getLastCommit(); ret.success && sha == ret.output.trimmed())
         {
            marks.append("detached");
            colors.append(graphDetached);
         }
      }

      const auto localBranches = mCache->getReferences(sha, References::Type::LocalBranch);
      for (const auto &branch : localBranches)
      {
         if (branch == currentBranch)
         {
            marks.prepend(branch);
            colors.prepend(graphCurrentBranch);
         }
         else
         {
            marks.append(branch);
            colors.append(graphLocalBranch);
         }
      }

      const auto tags = mCache->getReferences(sha, References::Type::LocalTag);
      for (const auto &tag : tags)
      {
         marks.append(tag);
         colors.append(graphTag);
      }

      const auto remoteBranches = mCache->getReferences(sha, References::Type::RemoteBranches);
      for (const auto &branch : remoteBranches)
      {
         marks.append(branch);
         colors.append(graphRemoteBranch);
      }

      const auto showMinimal = o.rect.width() <= MIN_VIEW_WIDTH_PX;
      const auto mark_spacing = 5; // Space between markers in pixels
      const auto mapEnd = marks.constEnd();

      auto mapIt = marks.constBegin();
      auto colorIter = colors.constBegin();
      for (; mapIt != mapEnd; ++mapIt, ++colorIter)
      {
         const auto isCurrentSpot = *mapIt == "detached" || *mapIt == currentBranch;
         o.font.setBold(isCurrentSpot);

         const auto nameToDisplay = showMinimal ? QString(". . .") : *mapIt;
         const QFontMetrics fm(o.font);
         const auto textBoundingRect = fm.boundingRect(nameToDisplay);
         const int textPadding = 3;
         const auto rectWidth = textBoundingRect.width() + 2 * textPadding;
         const QRectF markerRect(o.rect.x() + startPoint, o.rect.y() + 2, rectWidth, ROW_HEIGHT - 4);

         painter->save();
         painter->setRenderHint(QPainter::Antialiasing);
         painter->setPen(QPen(*colorIter, 2));
         QPainterPath path;
         path.addRoundedRect(markerRect, 1, 1);
         painter->fillPath(path, *colorIter);
         painter->drawPath(path);

         // TODO: Fix this with a nicer way
         painter->setPen(QColor(*colorIter == graphTag ? textColorBright : textColorDark));

         painter->setFont(o.font);
         painter->drawText(markerRect, Qt::AlignCenter, nameToDisplay);
         painter->restore();

         startPoint += rectWidth + mark_spacing;
      }
   }
}

void RepositoryViewDelegate::paintPrStatus(QPainter *painter, QStyleOptionViewItem opt, int &startPoint,
                                           const PullRequest &pr) const
{
   QColor c;

   switch (pr.state.eState)
   {
      case PullRequest::HeadState::State::Failure:
         c = GitQlientStyles::getRed();
         break;
      case PullRequest::HeadState::State::Success:
         c = GitQlientStyles::getGreen();
         break;
      default:
      case PullRequest::HeadState::State::Pending:
         c = GitQlientStyles::getOrange();
         break;
   }

   painter->save();
   painter->setRenderHint(QPainter::Antialiasing);
   painter->setPen(c);
   painter->setBrush(c);
   painter->drawEllipse(opt.rect.x() + startPoint, opt.rect.y() + (opt.rect.height() / 2) - 5, 10, 10);
   painter->restore();

   startPoint += 10 + 5;
}
