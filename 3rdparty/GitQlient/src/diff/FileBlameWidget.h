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
#include <QDateTime>

class GitBase;
class QScrollArea;
class ButtonLink;
class QLabel;
class GitCache;

/*!
 \brief The FileBalmeWidget class is the widget that creates the view for the blame of a file. It is formed by two
 parts: in the right side it is shown the code of the file. In the right and for every different line, it is shown the
 information of the commit with a color guide. The bright color indicates the more recent changes whereas the darkest
 color indicates the oldest.

*/
class FileBlameWidget : public QFrame
{
   Q_OBJECT

signals:
   /*!
    \brief Signal triggered when the user selects a revision in the blame.

    \param sha
   */
   void signalCommitSelected(const QString &sha);

public:
   /*!
    \brief Default constructor.

    \param cache The internal repository cache.
    \param git The git object to perform Git operations.
    \param parent The parent widget if needed.
   */
   explicit FileBlameWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                            QWidget *parent = nullptr);

   /*!
    \brief Sets up the widget by providing the file to blame and the last commit SHA where the file was modified. The
    previous sha is passed for general information.

    \param fileName The file name to blame.
    \param currentSha The last commit SHA where the file was modified.
    \param previousSha The previous commit SHA where the file was modified.
   */
   void setup(const QString &fileName, const QString &currentSha, const QString &previousSha);
   /*!
    \brief Reloads the current file blame information based on the new \p currentSha. The
    previous sha is passed for general information.

    \param currentSha A commit SHA where the file was modified.
    \param previousSha The previous commit SHA where the file was modified.
   */
   void reload(const QString &currentSha, const QString &previousSha);
   /*!
    \brief Retrieves the current used SHA.

    \return QString The current SHA.
   */
   QString getCurrentSha() const;
   /*!
    \brief Retrieves the file displayed.

    \return QString The file being displayed.
   */
   QString getCurrentFile() const { return mCurrentFile; }

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QFrame *mAnotation = nullptr;
   QLabel *mCurrentSha = nullptr;
   QLabel *mPreviousSha = nullptr;
   QScrollArea *mScrollArea = nullptr;
   QFont mInfoFont;
   QFont mCodeFont;
   QString mCurrentFile;

   /*!
    \brief Private class that stores data of a annotation. An annotation is the informatio regarding when a line was
    last modified.

   */
   struct Annotation
   {
      QString sha;
      QString author;
      QDateTime dateTime;
      int line = 0;
      QString content;
   };

   /*!
    \brief Processes a blame converting the git output into a vector of annotations per each line.

    \param blame The git blame output.
    \return QVector<Annotation> Vector of the annotations for every line.
   */
   QVector<Annotation> processBlame(const QString &blame);
   /*!
    \brief Process all the \p annotations and creates the view of the file with that information.

    \param annotations The annotations to process.
   */
   void formatAnnotatedFile(const QVector<Annotation> &annotations);
   /*!
    \brief Factory method that creates a label with the date and time based on an annotation.

    \param annotation The annotation to process.
    \param isFirst Indicates if it's the first item in the blame.
    \return QLabel Returns a newly created QLabel.
   */
   QLabel *createDateLabel(const Annotation &annotation, bool isFirst);
   /*!
    \brief Factory method that creates a label with the author information based on an annotation.

    \param author The author to be shown.
    \param isFirst Indicates if it's the first item in the blame.
    \return QLabel Returns a newly created QLabel.
   */
   QLabel *createAuthorLabel(const QString &author, bool isFirst);
   /*!
    \brief Factory method that creates a Clickable frame with the informatio of the commit (SHA and title).

    \param sha The sha to show.
    \param isFirst Indicates if it's the first item in the blame.
    \return Returns a newly created ButtonLink.
   */
   ButtonLink *createMessageLabel(const QString &sha, bool isFirst);
   /*!
    \brief Factory method that creates a label with the number of line. Uses the \p annotation parameter to display a
    visual help about when the change was done.

    \param annotation The annotation to process.
    \param row The row to display.
    \return QLabel Returns a newly created QLabel.
   */
   QLabel *createNumLabel(const Annotation &annotation, int row);
   /*!
    \brief Factory method that creates a label with the code line to be displayed.

    \param content The code line to display.
    \return QLabel Returns a newly created QLable.
   */
   QLabel *createCodeLabel(const QString &content);
};
