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

#include <PullRequest.h>

#include <QDialog>

namespace Ui
{
class MergePullRequestDlg;
}

class GitBase;

namespace GitServer
{
class IRestApi;
struct PullRequest;
}

/**
 * @brief The MergePullRequestDlg class creates the layout for the UI so the user can merge pull requests directly from
 * GitQlient.
 */
class MergePullRequestDlg : public QDialog
{
   Q_OBJECT

signals:
   /*!
    \brief Signal triggered when some action in the context menu things the main UI needs an update.
   */
   void signalRepositoryUpdated();

public:
   /**
    * @brief MergePullRequestDlg Detail constructor.
    * @param git The git object to perform Git operations.
    * @param pr The pull request to be merged.
    * @param sha The sha of the current commit to check that is the current sha in the server.
    * @param parent The parent widget.
    */
   explicit MergePullRequestDlg(const QSharedPointer<GitBase> git, const GitServer::PullRequest &pr, const QString &sha,
                                QWidget *parent = nullptr);
   /**
    * Destructor
    */
   ~MergePullRequestDlg();

private:
   Ui::MergePullRequestDlg *ui;
   QSharedPointer<GitBase> mGit;
   GitServer::PullRequest mPr;
   QString mSha;
   GitServer::IRestApi *mApi;

   /**
    * @brief accept Checks the data introduced by the user and triggers the connection against the server.
    */
   void accept() override;
   /**
    * @brief onPRMerged When the pull request has been merged, this method creates a message box that informs the user
    * about it.
    */
   void onPRMerged();
   /**
    * @brief onGitServerError Notifies the user that an error happened in the API connection or data exchange.
    */
   void onGitServerError(const QString &error);
};
