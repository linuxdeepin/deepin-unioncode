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

#include <QFrame>

#include <document.h>

namespace Ui
{
class CreateIssueDlg;
}

class GitServerCache;

namespace GitServer
{
struct Label;
struct Milestone;
struct Issue;
}

/**
 * @brief The CreateIssueDlg class presents the UI where the user can create issues in the remote Git server.
 */
class CreateIssueDlg : public QFrame
{
   Q_OBJECT

signals:
   void issueCreated();

public:
   /**
    * @brief CreateIssueDlg Default constructor.
    * @param git The git object to perform Git operations.
    * @param parent The parent widget.
    */
   explicit CreateIssueDlg(const QSharedPointer<GitServerCache> &gitServerCache, const QString &workingDir,
                           QWidget *parent = nullptr);

   /**
    * Destructor.
    */
   ~CreateIssueDlg();

   bool configure(const QString &workingDir);

private:
   Ui::CreateIssueDlg *ui;
   QSharedPointer<GitServerCache> mGitServerCache;
   Document m_content;

   /**
    * @brief accept Process the user input data and does a request to create an issue.
    */
   void accept();
   /**
    * @brief onMilestones Process the reply from the server when the milestones request is done.
    * @param milestones The list of milestones to process.
    */
   void onMilestones(const QVector<GitServer::Milestone> &milestones);
   /**
    * @brief onLabels Process the reply from the server when the labels request is done.
    * @param labels The list of labels to process.
    */
   void onLabels(const QVector<GitServer::Label> &labels);
   /**
    * @brief onIssueCreated Process the reply from the server if the issue was successfully created. It shows a message
    * box with the url of the issue.
    * @param url The url of the issue.
    */
   void onIssueCreated(const GitServer::Issue &issue);

   /**
    * @brief onGitServerError Notifies the user that an error happened in the API connection or data exchange.
    */
   void onGitServerError(const QString &error);

   void fillIssueTypeComboBox(const QString &workingDir);

   void onIssueTemplateChange(int newIndex);

   void updateMarkdown(const QByteArray &fileContent);
};
