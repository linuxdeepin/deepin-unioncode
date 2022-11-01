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

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QVector>

namespace QLogger
{

class QLoggerWriter : public QThread
{
   Q_OBJECT

public:
   /**
    * @brief Constructor that gets the complete path and filename to create the file. It also
    * configures the level of this file to filter the logs depending on the level.
    * @param fileDestination The complete path.
    * @param level The maximum level that is allowed.
    * @param fileFolderDestination The complete folder destination.
    * @param mode The logging mode.
    * @param fileSuffixIfFull The filename suffix if the file is full.
    */
   explicit QLoggerWriter(const QString &fileDestination, LogLevel level = LogLevel::Warning,
                          const QString &fileFolderDestination = QString(), LogMode mode = LogMode::OnlyFile,
                          LogFileDisplay fileSuffixIfFull = LogFileDisplay::DateTime,
                          LogMessageDisplays messageOptions = LogMessageDisplay::Default);

   /**
    * @brief Gets path and folder of the file that will store the logs.
    */
   QString getFileDestinationFolder() const { return mFileDestinationFolder; }
   /**
    * @brief Path and name of the file that will store the logs.
    */
   QString getFileDestination() const { return mFileDestination; }

   /**
    * @brief Gets the current logging mode.
    * @return The level.
    */
   LogMode getMode() const { return mMode; }

   /**
    * @brief setLogMode Sets the log mode for this destination.
    * @param mode
    */
   void setLogMode(LogMode mode);

   /**
    * @brief Gets the current level threshold.
    * @return The level.
    */
   LogLevel getLevel() const { return mLevel; }

   /**
    * @brief setLogLevel Sets the log level for this destination.
    * @param level The new level threshold.
    */
   void setLogLevel(LogLevel level) { mLevel = level; }

   /**
    * @brief Gets the current max size for the log file.
    * @return The maximum size
    */
   int getMaxFileSize() const { return mMaxFileSize; }

   /**
    * @brief setMaxFileSize Sets the max file size for this destination.
    * @param maxSize The new file size
    */
   void setMaxFileSize(int maxSize) { mMaxFileSize = maxSize; }

   /**
    * @brief getMessageOptions Gets the current message options.
    * @return The current options
    */
   LogMessageDisplays getMessageOptions() const { return mMessageOptions; }

   /**
    * @brief setMessageOptions Specifies what elements are displayed in one line of log message.
    * @param messageOptions The options
    */
   void setMessageOptions(LogMessageDisplays messageOptions) { mMessageOptions = messageOptions; }

   /**
    * @brief enqueue Enqueues a message to be written in the destiantion.
    * @param date The date and time of the log message.
    * @param threadId The thread where the message comes from.
    * @param module The module that writes the message.
    * @param level The log level of the message.
    * @param function The function that prints the log.
    * @param fileName The file name that prints the log.
    * @param line The line of the file name that prints the log.
    * @param message The message to log.
    */
   void enqueue(const QDateTime &date, const QString &threadId, const QString &module, LogLevel level,
                const QString &function, const QString &fileName, int line, const QString &message);

   /**
    * @brief Stops the log writer
    * @param stop True to be stop, otherwise false
    */
   void stop(bool stop) { mIsStop = stop; }

   /**
    * @brief Returns if the log writer is stop from writing.
    * @return True if is stop, otherwise false
    */
   bool isStop() const { return mIsStop; }

   /**
    * @brief run Overloaded method from QThread used to wait for new messages.
    */
   void run() override;

   /**
    * @brief closeDestination Closes the destination. This needs to be called whenever
    */
   void closeDestination();

private:
   bool mQuit = false;
   bool mIsStop = false;
   QWaitCondition mQueueNotEmpty;
   QString mFileDestinationFolder;
   QString mFileDestination;
   LogFileDisplay mFileSuffixIfFull;
   LogMode mMode;
   LogLevel mLevel;
   int mMaxFileSize = 1024 * 1024; //! @note 1Mio
   LogMessageDisplays mMessageOptions;
   QVector<QString> mMessages;
   QMutex mutex;

   /**
    * @brief renameFileIfFull Truncates the log file in two. Keeps the filename for the new one and renames the old one
    * with the timestamp or with a file number.
    *
    * @return Returns the file name for the old logs.
    */
   QString renameFileIfFull();

   /**
    * @brief generateDuplicateFilename
    *
    * @param fileDestination The file path and name without the extension.
    * @param fileExtension The file extension
    * @param fileSuffixNumber The file suffix number.
    * @return The complete path of the duplicated file name.
    */
   static QString generateDuplicateFilename(const QString &fileDestination, const QString &fileExtension,
                                            int fileSuffixNumber = 1);

   /**
    * @brief Writes a message in a file. If the file is full, it truncates it and prints a first line with the
    * information of the old file.
    *
    * @param message Pair of values consistent on the date and the message to be log.
    */
   void write(QVector<QString> messages);
};

}
