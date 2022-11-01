#include "GitQlientSettings.h"

#include <QVector>

QString GitQlientSettings::PinnedRepos = "Config/PinnedRepos";
QString GitQlientSettings::SplitFileDiffView = "SplitDiff";

GitQlientSettings::GitQlientSettings(const QString &gitRepoPath)
   : mGitRepoPath(gitRepoPath)
{
}

void GitQlientSettings::setGlobalValue(const QString &key, const QVariant &value)
{
   globalSettings.setValue(key, value);
   globalSettings.sync();
}

QVariant GitQlientSettings::globalValue(const QString &key, const QVariant &defaultValue)
{
   return globalSettings.value(key, defaultValue);
}

void GitQlientSettings::setLocalValue(const QString &key, const QVariant &value)
{
   QSettings localSettings(mGitRepoPath + "/GitQlientConfig.ini", QSettings::IniFormat);
   localSettings.setValue(key, value);
   localSettings.sync();
}

QVariant GitQlientSettings::localValue(const QString &key, const QVariant &defaultValue)
{
   QSettings localSettings(mGitRepoPath + "/GitQlientConfig.ini", QSettings::IniFormat);
   return localSettings.value(key, defaultValue);
}

void GitQlientSettings::setProjectOpened(const QString &projectPath)
{
   saveMostUsedProjects(projectPath);

   saveRecentProjects(projectPath);
}

QStringList GitQlientSettings::getRecentProjects() const
{
   auto projects = globalSettings.value("Config/RecentProjects", QStringList()).toStringList();

   QStringList recentProjects;
   const auto end = std::min(projects.count(), 5);

   for (auto i = 0; i < end; ++i)
      recentProjects.append(projects.takeFirst());

   return recentProjects;
}

void GitQlientSettings::saveRecentProjects(const QString &projectPath)
{
   auto usedProjects = globalSettings.value("Config/RecentProjects", QStringList()).toStringList();

   if (usedProjects.contains(projectPath))
   {
      const auto index = usedProjects.indexOf(projectPath);
      usedProjects.takeAt(index);
   }

   usedProjects.prepend(projectPath);

   while (!usedProjects.isEmpty() && usedProjects.count() > 5)
      usedProjects.removeLast();

   GitQlientSettings::setGlobalValue("Config/RecentProjects", usedProjects);
}

void GitQlientSettings::clearRecentProjects()
{
   globalSettings.remove("Config/RecentProjects");
}

void GitQlientSettings::saveMostUsedProjects(const QString &projectPath)
{
   auto projects = globalSettings.value("Config/UsedProjects", QStringList()).toStringList();
   auto timesUsed = globalSettings.value("Config/UsedProjectsCount", QList<QVariant>()).toList();

   if (projects.contains(projectPath))
   {
      const auto index = projects.indexOf(projectPath);
      timesUsed[index] = QString::number(timesUsed[index].toInt() + 1);
   }
   else
   {
      projects.append(projectPath);
      timesUsed.append(1);
   }

   GitQlientSettings::setGlobalValue("Config/UsedProjects", projects);
   GitQlientSettings::setGlobalValue("Config/UsedProjectsCount", timesUsed);
}

void GitQlientSettings::clearMostUsedProjects()
{
   globalSettings.remove("Config/UsedProjects");
   globalSettings.remove("Config/UsedProjectsCount");
}

QStringList GitQlientSettings::getMostUsedProjects() const
{
   const auto projects = globalSettings.value("Config/UsedProjects", QStringList()).toStringList();
   const auto timesUsed = globalSettings.value("Config/UsedProjectsCount", QString()).toList();

   QMultiMap<int, QString> projectOrderedByUse;

   const auto projectsCount = projects.count();
   const auto timesCount = timesUsed.count();

   for (auto i = 0; i < projectsCount && i < timesCount; ++i)
      projectOrderedByUse.insert(timesUsed.at(i).toInt(), projects.at(i));

   QStringList recentProjects;
   const auto end = std::min(projectOrderedByUse.count(), 5);
   const auto orderedProjects = projectOrderedByUse.values();

   for (auto i = 0; i < end; ++i)
      recentProjects.append(orderedProjects.at(orderedProjects.count() - 1 - i));

   return recentProjects;
}
