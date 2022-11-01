#pragma once

#include <Colors.h>

#include <QDateTime>
#include <QString>
#include <QMap>
#include <QMetaType>
#include <QColor>
#include <QVariant>
#include <QVector>

namespace Jenkins
{

inline QColor resultColor(const QString &result)
{
   if (result == "SUCCESS")
      return jenkinsResultSuccess;
   else if (result == "UNSTABLE")
      return gitQlientOrange;
   else if (result == "FAILURE" || result == "FAILED")
      return jenkinsResultFailure;
   else if (result == "ABORTED")
      return jenkinsResultAborted;
   else
      return jenkinsResultNotBuilt;
}

struct JenkinsStageInfo
{
   int id;
   QString name;
   QString url;
   int duration;
   QString result;
};

struct JenkinsJobBuildInfo
{
   struct Artifact
   {
      QString fileName;
      QString url;
   };

   bool operator==(const JenkinsJobBuildInfo &build) const { return url == build.url; }

   int number;
   QString url;
   QDateTime date;
   int duration;
   QString result;
   QString user;
   QVector<Artifact> artifacts;
   QVector<JenkinsStageInfo> stages;
};

enum class JobConfigFieldType
{
   Bool,
   String,
   Choice
};

struct JenkinsJobBuildConfig
{
   JobConfigFieldType fieldType;
   QString name;
   QVariant defaultValue;
   QStringList choicesValues;
};

struct JenkinsJobInfo
{
   bool operator==(const JenkinsJobInfo &info) const { return name == info.name; }
   bool operator!=(const JenkinsJobInfo &info) const { return !(name == info.name); }
   bool operator<(const JenkinsJobInfo &info) const { return name < info.name; }

   struct HealthStatus
   {
      QString score;
      QString description;
      QString iconClassName;
   };

   QString name;
   QString url;
   QString color;
   bool buildable;
   bool inQueue;
   HealthStatus healthStatus;
   QVector<JenkinsJobBuildInfo> builds;
   QVector<JenkinsJobBuildConfig> configFields;
};

}

Q_DECLARE_METATYPE(Jenkins::JenkinsJobInfo);
