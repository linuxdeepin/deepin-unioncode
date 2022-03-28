#include "Lane.h"

#include <LaneType.h>

Lane::Lane(LaneType type)
   : mType(type)
{
}

bool Lane::isHead() const
{
   return mType == LaneType::HEAD || mType == LaneType::HEAD_R || mType == LaneType::HEAD_L;
}

bool Lane::isTail() const
{
   return mType == LaneType::TAIL || mType == LaneType::TAIL_R || mType == LaneType::TAIL_L;
}

bool Lane::isJoin() const
{
   return mType == LaneType::JOIN || mType == LaneType::JOIN_R || mType == LaneType::JOIN_L;
}

bool Lane::isFreeLane() const
{
   return mType == LaneType::NOT_ACTIVE || mType == LaneType::CROSS || isJoin();
}

bool Lane::isMerge() const
{
   return mType == LaneType::MERGE_FORK || mType == LaneType::MERGE_FORK_R || mType == LaneType::MERGE_FORK_L;
}

bool Lane::isActive() const
{
   return mType == LaneType::ACTIVE || mType == LaneType::INITIAL || mType == LaneType::BRANCH || isMerge();
}
