#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <Issue.h>
#include <Commit.h>

#include <QMap>

namespace GitServer
{

struct PullRequest : public Issue
{
   PullRequest() = default;

   PullRequest(const Issue &issue)
   {
      number = issue.number;
      title = issue.title;
      body = issue.body;
      milestone = issue.milestone;
      labels = issue.labels;
      creator = issue.creator;
      assignees = issue.assignees;
      url = issue.url;
      creation = issue.creation;
      comments = issue.comments;
   }

   struct HeadState
   {
      enum class State
      {
         Failure,
         Success,
         Pending
      };

      struct Check
      {
         QString description;
         QString state;
         QString url;
         QString name;
      };

      QString sha {};
      QString state {};
      State eState {};
      QVector<Check> checks {};
   };

   QString head {};
   QString headRepo {};
   QString headUrl {};
   QString base {};
   QString baseRepo {};
   bool maintainerCanModify = true;
   bool draft = false;
   int id = 0;
   QString url {};
   HeadState state {};
   QMap<int, Review> reviews {};
   QVector<CodeReview> reviewComment {};
   int reviewCommentsCount = 0;
   int commitCount = 0;
   int additions = 0;
   int deletions = 0;
   int changedFiles = 0;
   bool merged = false;
   bool mergeable = false;
   bool rebaseable = false;
   QString mergeableState {};
   QVector<Commit> commits {};

   QJsonObject toJson() const
   {
      QJsonObject object;

      object.insert("title", title);
      object.insert("head", head);
      object.insert("base", base);
      object.insert("body", body.toStdString().c_str());
      object.insert("maintainer_can_modify", maintainerCanModify);
      object.insert("draft", draft);

      return object;
   }

   bool isValid() const { return !title.isEmpty(); }

public:
   int getCommits() const;
   void setCommits(int value);
};

}
