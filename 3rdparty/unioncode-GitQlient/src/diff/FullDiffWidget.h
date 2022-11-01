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

#include <IDiffWidget.h>

#include <QSyntaxHighlighter>

class QPlainTextEdit;
class QPushButton;

/*!
 \brief The FullDiffWidget class is an overload class inherited from QTextEdit that process the output from a diff for a
 full commit diff. It includes a highlighter for the lines that are added, removed and to differentiate where a file
 diff chuck starts.

*/
class FullDiffWidget : public IDiffWidget
{
   Q_OBJECT

public:
   /*!
    \brief Default constructor.

    \param git The git object to perform Git operations.
    \param parent The parent widget if needed.
   */
   explicit FullDiffWidget(const QSharedPointer<GitBase> &git, QSharedPointer<GitCache> cache,
                           QWidget *parent = nullptr);

   /*!
    \brief Reloads the current diff in case the user loaded the work in progress as base commit.

   */
   bool reload() override;
   /*!
    \brief Loads a diff for a specific commit SHA respect another commit SHA.

    \param sha The base commit SHA.
    \param diffToSha The commit SHA to compare to.
    \param diffData The diff data returned by the git command.
    \return True if there is a diff to load, otherwise false.
   */
   void loadDiff(const QString &sha, const QString &diffToSha, const QString &diffData);

private:
   QPushButton *mGoPrevious = nullptr;
   QPushButton *mGoNext = nullptr;
   QString mPreviousDiffText;
   QPlainTextEdit *mDiffWidget = nullptr;
   QVector<int> mFilePositions;

   class DiffHighlighter : public QSyntaxHighlighter
   {
   public:
      DiffHighlighter(QTextDocument *document);
      void highlightBlock(const QString &text) override;
   };

   DiffHighlighter *diffHighlighter = nullptr;

   /*!
    \brief Method that processes the data from the Git diff command.

    \param fileChunk The file chuck to compare.
   */
   void processData(const QString &fileChunk);
   /**
    * @brief moveChunkUp Moves to the previous diff chunk.
    */
   void moveChunkUp();
   /**
    * @brief moveChunkDown Moves to the following diff chunk.
    */
   void moveChunkDown();
};
