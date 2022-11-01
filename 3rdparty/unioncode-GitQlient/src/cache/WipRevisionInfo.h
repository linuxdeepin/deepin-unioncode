#pragma once

#include <QString>

struct WipRevisionInfo
{
   QString parentSha;
   QString diffIndex;
   QString diffIndexCached;

   bool isValid() const { return !parentSha.isEmpty() || !diffIndex.isEmpty() || !diffIndexCached.isEmpty(); }
};
