#pragma once

enum class LaneType
{
   EMPTY,
   ACTIVE,
   NOT_ACTIVE,
   MERGE_FORK,
   MERGE_FORK_R,
   MERGE_FORK_L,
   JOIN,
   JOIN_R,
   JOIN_L,
   HEAD,
   HEAD_R,
   HEAD_L,
   TAIL,
   TAIL_R,
   TAIL_L,
   CROSS,
   CROSS_EMPTY,
   INITIAL,
   BRANCH,
   LANE_TYPES_NUM
};
