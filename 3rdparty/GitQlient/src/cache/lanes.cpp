/*
        Description: history graph computation

        Author: Marco Costalba (C) 2005-2007

        Copyright: See COPYING file that comes with this distribution

*/
#include "lanes.h"

#include <QStringList>

void Lanes::init(const QString &expectedSha)
{
   clear();
   activeLane = 0;
   add(LaneType::BRANCH, expectedSha, activeLane);
}

void Lanes::clear()
{
   typeVec.clear();
   typeVec.squeeze();
   nextShaVec.clear();
   nextShaVec.squeeze();
}

bool Lanes::isFork(const QString &sha, bool &isDiscontinuity)
{
   int pos = findNextSha(sha, 0);
   isDiscontinuity = activeLane != pos;

   return pos == -1 ? false : findNextSha(sha, pos + 1) != -1;
}

void Lanes::setFork(const QString &sha)
{
   auto rangeEnd = 0;
   auto idx = 0;
   auto rangeStart = rangeEnd = idx = findNextSha(sha, 0);

   while (idx != -1)
   {
      rangeEnd = idx;
      typeVec[idx].setType(LaneType::TAIL);
      idx = findNextSha(sha, idx + 1);
   }

   typeVec[activeLane].setType(NODE);

   auto &startT = typeVec[rangeStart];
   auto &endT = typeVec[rangeEnd];

   if (startT.equals(NODE))
      startT.setType(NODE_L);

   if (endT.equals(NODE))
      endT.setType(NODE_R);

   if (startT.equals(LaneType::TAIL))
      startT.setType(LaneType::TAIL_L);

   if (endT.equals(LaneType::TAIL))
      endT.setType(LaneType::TAIL_R);

   for (int i = rangeStart + 1; i < rangeEnd; ++i)
   {
      switch (auto &t = typeVec[i]; t.getType())
      {
         case LaneType::NOT_ACTIVE:
            t.setType(LaneType::CROSS);
            break;
         case LaneType::EMPTY:
            t.setType(LaneType::CROSS_EMPTY);
            break;
         default:
            break;
      }
   }
}

void Lanes::setMerge(const QStringList &parents)
{
   auto &t = typeVec[activeLane];
   auto wasFork = t.equals(NODE);
   auto wasFork_L = t.equals(NODE_L);
   auto wasFork_R = t.equals(NODE_R);
   auto startJoinWasACross = false;
   auto endJoinWasACross = false;

   t.setType(NODE);

   auto rangeStart = activeLane;
   auto rangeEnd = activeLane;
   QStringList::const_iterator it(parents.constBegin());

   for (++it; it != parents.constEnd(); ++it)
   { // skip first parent
      int idx = findNextSha(*it, 0);

      if (idx != -1)
      {
         if (idx > rangeEnd)
         {
            rangeEnd = idx;
            endJoinWasACross = typeVec[idx].equals(LaneType::CROSS);
         }

         if (idx < rangeStart)
         {
            rangeStart = idx;
            startJoinWasACross = typeVec[idx].equals(LaneType::CROSS);
         }

         typeVec[idx].setType(LaneType::JOIN);
      }
      else
         rangeEnd = add(LaneType::HEAD, *it, rangeEnd + 1);
   }

   auto &startT = typeVec[rangeStart];
   auto &endT = typeVec[rangeEnd];

   if (startT.equals(NODE) && !wasFork && !wasFork_R)
      startT.setType(NODE_L);

   if (endT.equals(NODE) && !wasFork && !wasFork_L)
      endT.setType(NODE_R);

   if (startT.equals(LaneType::JOIN) && !startJoinWasACross)
      startT.setType(LaneType::JOIN_L);

   if (endT.equals(LaneType::JOIN) && !endJoinWasACross)
      endT.setType(LaneType::JOIN_R);

   if (startT.equals(LaneType::HEAD))
      startT.setType(LaneType::HEAD_L);

   if (endT.equals(LaneType::HEAD))
      endT.setType(LaneType::HEAD_R);

   for (int i = rangeStart + 1; i < rangeEnd; i++)
   {
      auto &t = typeVec[i];

      if (t.equals(LaneType::NOT_ACTIVE))
         t.setType(LaneType::CROSS);
      else if (t.equals(LaneType::EMPTY))
         t.setType(LaneType::CROSS_EMPTY);
      else if (t.equals(LaneType::TAIL_R) || t.equals(LaneType::TAIL_L))
         t.setType(LaneType::TAIL);
   }
}

void Lanes::setInitial()
{
   auto &t = typeVec[activeLane];

   if (!isNode(t))
      t.setType(LaneType::INITIAL);
}

void Lanes::changeActiveLane(const QString &sha)
{
   auto &t = typeVec[activeLane];

   if (t.equals(LaneType::INITIAL))
      t.setType(LaneType::EMPTY);
   else
      t.setType(LaneType::NOT_ACTIVE);

   int idx = findNextSha(sha, 0);
   if (idx != -1)
      typeVec[idx].setType(LaneType::ACTIVE);
   else
      idx = add(LaneType::BRANCH, sha, activeLane);

   activeLane = idx;
}

void Lanes::afterMerge()
{
   for (int i = 0; i < typeVec.count(); i++)
   {
      auto &t = typeVec[i];

      if (t.isHead() || t.isJoin() || t.equals(LaneType::CROSS))
         t.setType(LaneType::NOT_ACTIVE);
      else if (t.equals(LaneType::CROSS_EMPTY))
         t.setType(LaneType::EMPTY);
      else if (isNode(t))
         t.setType(LaneType::ACTIVE);
   }
}

void Lanes::afterFork()
{
   for (int i = 0; i < typeVec.count(); i++)
   {
      auto &t = typeVec[i];

      if (t.equals(LaneType::CROSS))
         t.setType(LaneType::NOT_ACTIVE);
      else if (t.isTail() || t.equals(LaneType::CROSS_EMPTY))
         t.setType(LaneType::EMPTY);

      if (isNode(t))
         t.setType(LaneType::ACTIVE);
   }

   while (typeVec.last().equals(LaneType::EMPTY))
   {
      typeVec.pop_back();
      nextShaVec.pop_back();
   }
}

bool Lanes::isBranch()
{
   if (typeVec.count() > activeLane)
      return typeVec.at(activeLane).equals(LaneType::BRANCH);

   return false;
}

void Lanes::afterBranch()
{
   typeVec[activeLane].setType(LaneType::ACTIVE);
}

void Lanes::nextParent(const QString &sha)
{
   nextShaVec[activeLane] = sha;
}

int Lanes::findNextSha(const QString &next, int pos)
{
   for (int i = pos; i < nextShaVec.count(); i++)
   {
      if (nextShaVec[i] == next)
         return i;
   }

   return -1;
}

int Lanes::findType(const LaneType type, int pos)
{
   const auto typeVecCount = typeVec.count();

   for (int i = pos; i < typeVecCount; i++)
   {
      if (typeVec[i].equals(type))
         return i;
   }

   return -1;
}

int Lanes::add(const LaneType type, const QString &next, int pos)
{
   if (pos < typeVec.count())
   {
      pos = findType(LaneType::EMPTY, pos);
      if (pos != -1)
      {
         typeVec[pos].setType(type);
         nextShaVec[pos] = next;
         return pos;
      }
   }

   typeVec.append(type);
   nextShaVec.append(next);
   return typeVec.count() - 1;
}

bool Lanes::isNode(Lane lane) const
{
   return lane.equals(NODE) || lane.equals(NODE_R) || lane.equals(NODE_L);
}
