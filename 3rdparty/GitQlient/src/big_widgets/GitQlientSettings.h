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

#include <QSettings>
#include <QVector>

/*!
 \brief The GitQlientSettings is an overloaded implementation of the QSettings that tries to help the user when a config
 parameter is modified by triggering a signal to notify the UI.

*/
class GitQlientSettings
{
public:
   /*!
    \brief Default constructor.

   */
   GitQlientSettings() = default;
   GitQlientSettings(const QString &gitRepoPath);
   ~GitQlientSettings() { }

   /*!
    \brief Sets a value for a given \p key.

    \param key The key.
    \param value The new value for the key.
   */
   void setGlobalValue(const QString &key, const QVariant &value);

   /**
    * @brief getGlobalValue Returns the value for a given @p key.
    * @param key The key.
    * @param defaultValue (optional) A default value in case the key doesn't exist.
    */
   QVariant globalValue(const QString &key, const QVariant &defaultValue = QVariant());

   /**
    * @brief setLocalValue Sets a value for a given @p repo with a @p key and @p value.
    * @param repo The local repo to store the config value.
    * @param key The key.
    * @param value The new value for the key.
    */
   void setLocalValue(const QString &key, const QVariant &value);

   /**
    * @brief getLocalValue Returns the value for a given @p repo and a given @p key.
    * @param repo The repo to retrieve where the key from.
    * @param key The key
    * @param defaultValue (optional) A default value in case the key doesn't exist.
    */
   QVariant localValue(const QString &key, const QVariant &defaultValue = QVariant());

   /*!
    \brief Stores that a project is opened. This is used to recalculate which projects are the most used.

    \param projectPath The project path of the repository.
   */
   void setProjectOpened(const QString &projectPath);
   /*!
    * \brief Gets all the recent used projects.
    *
    * \return QStringList Projects list.
    */
   QStringList getRecentProjects() const;
   /*!
    * \brief saveRecentProjects Saves the project in \p projectPath in the recent projects config value.
    * \param projectPath The project path to save.
    */
   void saveRecentProjects(const QString &projectPath);

   /**
    * @brief clearRecentProjects Clears the recent projects list.
    */
   void clearRecentProjects();
   /*!
    * \brief saveMostUsedProjects Saves the project in \p projectPath in the most used projects config value.
    * \param projectPath The project path to save.
    */
   void saveMostUsedProjects(const QString &projectPath);

   /**
    * @brief clearMostUsedProjects Clears the most used projects list.
    */
   void clearMostUsedProjects();

   /*!
    \brief Gets all the most used projects.

    \return QStringList Projects list.
    */
   QStringList getMostUsedProjects() const;

   static QString PinnedRepos;
   static QString SplitFileDiffView;

private:
   QSettings globalSettings;
   QString mGitRepoPath;
};
