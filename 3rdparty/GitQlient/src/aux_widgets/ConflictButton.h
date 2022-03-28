#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This library is free software; you can redistribute it and/or
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

class GitBase;
class QPushButton;

/**
 * @brief The ConflictButton class creates buttons that are used by the MergeWidget. The button is composed by three
 * different QPushButtons. The first one shows the file name and allows the user to change the selection when clicks
 * over it. Next to it there is the update button that allows the user to refresh the view in the merge widget. The last
 * button is the resolve button. This button allows the user to mark the merge conflict in the the file as solved and
 * adds the file to the commit.
 *
 */
class ConflictButton : public QFrame
{
   Q_OBJECT

signals:
   /**
    * @brief Signal triggered when the name is clicked.
    *
    * @param checked True if the button is selected, otherwise false.
    */
   void clicked();
   /**
    * @brief Signal triggered when the user solves the merge conflict.
    *
    */
   void resolved();
   /**
    * @brief Signal triggered when the user requests an update of the file content.
    *
    */
   void updateRequested();

   /**
    * @brief signalEditFile Signal triggered when the user wants to edit a file and is running GitQlient from QtCreator.
    * @param fileName The file name
    * @param line The line
    * @param column The column
    */
   void signalEditFile(const QString &fileName, int line, int column);

public:
   /**
    * @brief Default constructor.
    *
    * @param filename The file name.
    * @param inConflict Indicates if the file has conflicts.
    * @param git The git object to perform Git operations.
    * @param parent The parent wiget if needed.
    */
   explicit ConflictButton(const QString &filename, bool inConflict, const QSharedPointer<GitBase> &git,
                           QWidget *parent = nullptr);

   /**
    * @brief Sets the button as selected.
    *
    * @param checked The new check state.
    */
   void setChecked(bool checked);

   /**
    * @brief getFileName
    * @return
    */
   QString getFileName() const;

private:
   QSharedPointer<GitBase> mGit;
   QString mFileName;
   QPushButton *mFile = nullptr;
   QPushButton *mEdit = nullptr;
   QPushButton *mResolve = nullptr;
   QPushButton *mUpdate = nullptr;

   /**
    * @brief Sets the button and the file as merge conflict.
    *
    * @param inConflict True if it is in conflict, otherwise false.
    */
   void setInConflict(bool inConflict);
   /**
    * @brief Resolves the conflict of the file and adds the file to the merge commit.
    *
    */
   void resolveConflict();
};
