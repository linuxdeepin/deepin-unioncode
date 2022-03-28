#pragma once

/****************************************************************************************
 ** QLogger is a library to register and print logs into a file.
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
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <QLoggerLevel.h>

#include <QMutex>
#include <QMap>
#include <QVariant>

namespace QLogger
{

class QLoggerWriter;

/**
 * @brief The QLoggerManager class manages the different destination files that we would like to have.
 */
class QLoggerManager
{
public:
   /**
    * @brief Gets an instance to the QLoggerManager.
    * @return A pointer to the instance.
    */
   static QLoggerManager *getInstance();

   /**
    * @brief This method creates a QLoogerWriter that stores the name of the file and the log
    * level assigned to it. Here is added to the map the different modules assigned to each
    * log file. The method returns <em>false</em> if a module is configured to be stored in
    * more than one file.
    *
    * @param fileDest The file name and path to print logs.
    * @param module The module that will be stored in the file.
    * @param level The maximum level allowed.
    * @param fileFolderDestination The complete folder destination.
    * @param mode The logging mode.
    * @param fileSuffixIfFull The filename suffix if the file is full.
    * @param messageOptions Specifies what elements are displayed in one line of log message.
    * @return Returns true if any error have been done.
    */
   bool addDestination(const QString &fileDest, const QString &module, LogLevel level = LogLevel::Warning,
                       const QString &fileFolderDestination = QString(), LogMode mode = LogMode::OnlyFile,
                       LogFileDisplay fileSuffixIfFull = LogFileDisplay::DateTime,
                       LogMessageDisplays messageOptions = LogMessageDisplay::Default, bool notify = true);
   /**
    * @brief This method creates a QLoogerWriter that stores the name of the file and the log
    * level assigned to it. Here is added to the map the different modules assigned to each
    * log file. The method returns <em>false</em> if a module is configured to be stored in
    * more than one file.
    *
    * @param fileDest The file name and path to print logs.
    * @param modules The modules that will be stored in the file.
    * @param level The maximum level allowed.
    * @param fileFolderDestination The complete folder destination.
    * @param mode The logging mode.
    * @param fileSuffixIfFull The filename suffix if the file is full.
    * @param messageOptions Specifies what elements are displayed in one line of log message.
    * @return Returns true if any error have been done.
    */
   bool addDestination(const QString &fileDest, const QStringList &modules, LogLevel level = LogLevel::Warning,
                       const QString &fileFolderDestination = QString(), LogMode mode = LogMode::OnlyFile,
                       LogFileDisplay fileSuffixIfFull = LogFileDisplay::DateTime,
                       LogMessageDisplays messageOptions = LogMessageDisplay::Default, bool notify = true);
   /**
    * @brief Clears old log files from the current storage folder.
    *
    * @param fileFolderDestination The destination folder.
    * @param days Minimum age of log files to delete. Logs older than
    *        this value will be removed. If days is -1, deletes any log file.
    */
   static void clearFileDestinationFolder(const QString &fileFolderDestination, int days = -1);
   /**
    * @brief enqueueMessage Enqueues a message in the corresponding QLoggerWritter.
    * @param module The module that writes the message.
    * @param level The level of the message.
    * @param message The message to log.
    * @param function The function in the file where the log comes from.
    * @param file The file that logs.
    * @param line The line in the file where the log comes from.
    */
   void enqueueMessage(const QString &module, LogLevel level, const QString &message, const QString &function,
                       const QString &file, int line);

   /**
    * @brief Whether the QLogger is paused or not.
    */
   bool isPaused() const { return mIsStop; }

   /**
    * @brief pause Pauses all QLoggerWriters.
    */
   void pause();

   /**
    * @brief resume Resumes all QLoggerWriters that where paused.
    */
   void resume();

   /**
    * @brief getDefaultFileDestinationFolder Gets the defaut file destination folder.
    * @return The file destination folder
    */
   QString getDefaultFileDestinationFolder() const { return mDefaultFileDestinationFolder; }

   /**
    * @brief getDefaultMode Gets the default log mode.
    * @return The log mode
    */
   LogMode getDefaultMode() const { return mDefaultMode; }

   /**
    * @brief getDefaultLevel Gets the default log level.
    * @return The log level
    */
   LogLevel getDefaultLevel() const { return mDefaultLevel; }

   /**
    * @brief Sets default values for QLoggerWritter parameters. Usefull for multiple QLoggerWritter.
    */
   void setDefaultFileDestinationFolder(const QString &fileDestinationFolder);
   void setDefaultFileDestination(const QString &fileDestination) { mDefaultFileDestination = fileDestination; }
   void setDefaultFileSuffixIfFull(LogFileDisplay fileSuffixIfFull) { mDefaultFileSuffixIfFull = fileSuffixIfFull; }

   void setDefaultLevel(LogLevel level) { mDefaultLevel = level; }
   void setDefaultMode(LogMode mode) { mDefaultMode = mode; }
   void setDefaultMaxFileSize(int maxFileSize) { mDefaultMaxFileSize = maxFileSize; }
   void setDefaultMessageOptions(LogMessageDisplays messageOptions) { mDefaultMessageOptions = messageOptions; }

   /**
    * @brief overwriteLogMode Overwrites the logging mode in all the destinations. Sets the default logging mode.
    *
    * @param mode The new log mode
    */
   void overwriteLogMode(LogMode mode);
   /**
    * @brief overwriteLogLevel Overwrites the log level in all the destinations. Sets the default log level.
    *
    * @param level The new log level
    */
   void overwriteLogLevel(LogLevel level);
   /**
    * @brief overwriteMaxFileSize Overwrites the maximum file size in all the destinations. Sets the default max file
    * size.
    *
    * @param maxSize The new file size
    */
   void overwriteMaxFileSize(int maxSize);

private:
   /**
    * @brief Checks if the logger is stop
    */
   bool mIsStop = false;

   /**
    * @brief Map that stores the module and the file it is assigned.
    */
   QMap<QString, QLoggerWriter *> mModuleDest;

   /**
    * @brief Defines the queue of messages when no writters have been set yet.
    */
   QMultiMap<QString, QVector<QVariant>> mNonWriterQueue;

   /**
    * @brief Default values for QLoggerWritter parameters. Usefull for multiple QLoggerWritter.
    */
   QString mDefaultFileDestinationFolder;
   QString mDefaultFileDestination;
   LogFileDisplay mDefaultFileSuffixIfFull = LogFileDisplay::DateTime;

   LogMode mDefaultMode = LogMode::OnlyFile;
   LogLevel mDefaultLevel = LogLevel::Warning;
   int mDefaultMaxFileSize = 1024 * 1024; //! @note 1Mio
   LogMessageDisplays mDefaultMessageOptions = LogMessageDisplay::Default;

   /**
    * @brief Mutex to make the method thread-safe.
    */
   QMutex mMutex { QMutex::Recursive };

   /**
    * @brief Default builder of the class. It starts the thread.
    */
   QLoggerManager() = default;

   /**
    * @brief Destructor
    */
   ~QLoggerManager();

   /**
    * @brief Initializes and returns a new instance of QLoggerWriter with the given parameters.
    * @param fileDest The file name and path to print logs.
    * @param level The maximum level allowed.
    * @param fileFolderDestination The complete folder destination.
    * @param mode The logging mode.
    * @param fileSuffixIfFull The filename suffix if the file is full.
    * @param messageOptions Specifies what elements are displayed in one line of log message.
    * @return the newly created QLoggerWriter instance.
    */
   QLoggerWriter *createWriter(const QString &fileDest, LogLevel level, const QString &fileFolderDestination,
                               LogMode mode, LogFileDisplay fileSuffixIfFull, LogMessageDisplays messageOptions) const;

   void startWriter(const QString &module, QLoggerWriter *log, LogMode mode, bool notify);

   /**
    * @brief Checks the queue and writes the messages if the writer is the correct one. The queue is emptied
    * for that module.
    * @param module The module to dequeue the messages from
    */
   void writeAndDequeueMessages(const QString &module);
};

/**
 * @brief Here is done the call to write the message in the module. First of all is confirmed
 * that the log level we want to write is less or equal to the level defined when we create the
 * destination.
 *
 * @param module The module that the message references.
 * @param level The level of the message.
 * @param message The message.
 * @param function The function in the file where the log comes from.
 * @param file The file that logs.
 * @param line The line in the file where the log comes from.
 */
extern void QLog_(const QString &module, QLogger::LogLevel level, const QString &message, const QString &function,
                  const QString &file = QString(), int line = -1);

}

#ifndef QLog_Trace
/**
 * @brief Used to store Trace level messages.
 * @param module The module that the message references.
 * @param message The message.
 */
#   define QLog_Trace(module, message)                                                                                 \
      QLogger::QLoggerManager::getInstance()->enqueueMessage(module, QLogger::LogLevel::Trace, message, __FUNCTION__,  \
                                                             __FILE__, __LINE__)
#endif

#ifndef QLog_Debug
/**
 * @brief Used to store Debug level messages.
 * @param module The module that the message references.
 * @param message The message.
 */
#   define QLog_Debug(module, message)                                                                                 \
      QLogger::QLoggerManager::getInstance()->enqueueMessage(module, QLogger::LogLevel::Debug, message, __FUNCTION__,  \
                                                             __FILE__, __LINE__)
#endif

#ifndef QLog_Info
/**
 * @brief Used to store Info level messages.
 * @param module The module that the message references.
 * @param message The message.
 */
#   define QLog_Info(module, message)                                                                                  \
      QLogger::QLoggerManager::getInstance()->enqueueMessage(module, QLogger::LogLevel::Info, message, __FUNCTION__,   \
                                                             __FILE__, __LINE__)
#endif

#ifndef QLog_Warning
/**
 * @brief Used to store Warning level messages.
 * @param module The module that the message references.
 * @param message The message.
 */
#   define QLog_Warning(module, message)                                                                               \
      QLogger::QLoggerManager::getInstance()->enqueueMessage(module, QLogger::LogLevel::Warning, message,              \
                                                             __FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef QLog_Error
/**
 * @brief Used to store Error level messages.
 * @param module The module that the message references.
 * @param message The message.
 */
#   define QLog_Error(module, message)                                                                                 \
      QLogger::QLoggerManager::getInstance()->enqueueMessage(module, QLogger::LogLevel::Error, message, __FUNCTION__,  \
                                                             __FILE__, __LINE__)
#endif

#ifndef QLog_Fatal
/**
 * @brief Used to store Fatal level messages.
 * @param module The module that the message references.
 * @param message The message.
 */
#   define QLog_Fatal(module, message)                                                                                 \
      QLogger::QLoggerManager::getInstance()->enqueueMessage(module, QLogger::LogLevel::Fatal, message, __FUNCTION__,  \
                                                             __FILE__, __LINE__)
#endif
