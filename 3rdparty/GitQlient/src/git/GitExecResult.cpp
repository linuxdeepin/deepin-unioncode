#include "GitExecResult.h"

GitExecResult::GitExecResult(bool ret, QString v)
   : success(ret)
   , output(std::move(v))
{
}

GitExecResult::GitExecResult(const QPair<bool, QString> &result)
   : success(result.first)
   , output(result.second)
{
}

GitExecResult &GitExecResult::operator=(const QPair<bool, QString> &result)
{
   success = result.first;
   output = result.second;

   return *this;
}
