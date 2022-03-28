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
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 ***************************************************************************************/

#include <QFrame>

class FileDiffEditor;
class Highlighter;

class FileEditor : public QFrame
{
   Q_OBJECT

signals:
   void signalEditionClosed();

public:
   explicit FileEditor(bool highlighter = true, QWidget *parent = nullptr);

   /**
    * @brief editFile Shows the file edition window with the content of
    * @p fileName loaded on it.
    * @param fileName The full path of the file that will be opened.
    */
   void editFile(const QString &fileName);

   /**
    * @brief finishEdition Tells the widget to finish the edition. If there are changes the widget asks the user if they
    * want to save them.
    */
   void finishEdition();

   /**
    * @brief saveFile Saves the current state of the file.
    */
   void saveFile();

private:
   FileDiffEditor *mFileEditor = nullptr;
   Highlighter *mHighlighter = nullptr;
   QString mFileName;
   QString mLoadedContent;
   bool isEditing = false;

   /**
    * @brief saveTextInFile Saves the current file.
    * @param content The content of the editor to be stored in the file.
    */
   void saveTextInFile(const QString &content) const;
};
