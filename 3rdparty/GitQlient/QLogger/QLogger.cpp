#include "QLogger.h"

#include "QLoggerWriter.h"

#include <QDateTime>
#include <QDir>

Q_DECLARE_METATYPE(QLogger::LogLevel);
Q_DECLARE_METATYPE(QLogger::LogMode);
Q_DECLARE_METATYPE(QLogger::LogFileDisplay);
Q_DECLARE_METATYPE(QLogger::LogMessageDisplay);

namespace QLogger
{

void QLog_(const QString &module, LogLevel level, const QString &message, const QString &function, const QString &file,
           int line)
{
   QLoggerManager::getInstance()->enqueueMessage(module, level, message, function, file, line);
}

static const int QUEUE_LIMIT = 100;

QLoggerManager *QLoggerManager::getInstance()
{
   static QLoggerManager INSTANCE;

   return &INSTANCE;
}

bool QLoggerManager::addDestination(const QString &fileDest, const QString &module, LogLevel level,
                                    const QString &fileFolderDestination, LogMode mode, LogFileDisplay fileSuffixIfFull,
                                    LogMessageDisplays messageOptions, bool notify)
{
   QMutexLocker lock(&mMutex);

   if (!mModuleDest.contains(module))
   {
      const auto log = createWriter(fileDest, level, fileFolderDestination, mode, fileSuffixIfFull, messageOptions);

      mModuleDest.insert(module, log);

      startWriter(module, log, mode, notify);

      return true;
   }

   return false;
}

bool QLoggerManager::addDestination(const QString &fileDest, const QStringList &modules, LogLevel level,
                                    const QString &fileFolderDestination, LogMode mode, LogFileDisplay fileSuffixIfFull,
                                    LogMessageDisplays messageOptions, bool notify)
{
   QMutexLocker lock(&mMutex);
   bool allAdded = false;

   for (const auto &module : modules)
   {
      if (!mModuleDest.contains(module))
      {
         const auto log = createWriter(fileDest, level, fileFolderDestination, mode, fileSuffixIfFull, messageOptions);

         mModuleDest.insert(module, log);

         startWriter(module, log, mode, notify);

         allAdded = true;
      }
   }

   return allAdded;
}

QLoggerWriter *QLoggerManager::createWriter(const QString &fileDest, LogLevel level,
                                            const QString &fileFolderDestination, LogMode mode,
                                            LogFileDisplay fileSuffixIfFull, LogMessageDisplays messageOptions) const
{
   const auto lFileDest = fileDest.isEmpty() ? mDefaultFileDestination : fileDest;
   const auto lLevel = level == LogLevel::Warning ? mDefaultLevel : level;
   const auto lFileFolderDestination = fileFolderDestination.isEmpty()
       ? mDefaultFileDestinationFolder
       : QDir::fromNativeSeparators(fileFolderDestination);
   const auto lMode = mode == LogMode::OnlyFile ? mDefaultMode : mode;
   const auto lFileSuffixIfFull
       = fileSuffixIfFull == LogFileDisplay::DateTime ? mDefaultFileSuffixIfFull : fileSuffixIfFull;
   const auto lMessageOptions
       = messageOptions.testFlag(LogMessageDisplay::Default) ? mDefaultMessageOptions : messageOptions;

   const auto log
       = new QLoggerWriter(lFileDest, lLevel, lFileFolderDestination, lMode, lFileSuffixIfFull, lMessageOptions);

   log->setMaxFileSize(mDefaultMaxFileSize);
   log->stop(mIsStop);

   return log;
}

void QLoggerManager::startWriter(const QString &module, QLoggerWriter *log, LogMode mode, bool notify)
{
   if (notify)
   {
      const auto threadId = QString("%1").arg((quintptr)QThread::currentThread(), QT_POINTER_SIZE * 2, 16, QChar('0'));
      log->enqueue(QDateTime::currentDateTime(), threadId, module, LogLevel::Info, "", "", -1, "Adding destination!");
   }

   if (mode != LogMode::Disabled)
      log->start();
}

void QLoggerManager::clearFileDestinationFolder(const QString &fileFolderDestination, int days)
{
   QDir dir(fileFolderDestination + QStringLiteral("/logs"));

   if (!dir.exists())
      return;

   dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

   const auto list = dir.entryInfoList();
   const auto now = QDateTime::currentDateTime();

   for (const auto &fileInfoIter : list)
   {
      if (fileInfoIter.lastModified().daysTo(now) >= days)
      {
         // remove file
         dir.remove(fileInfoIter.fileName());
      }
   }
}

void QLoggerManager::setDefaultFileDestinationFolder(const QString &fileDestinationFolder)
{
   mDefaultFileDestinationFolder = QDir::fromNativeSeparators(fileDestinationFolder);
}

void QLoggerManager::writeAndDequeueMessages(const QString &module)
{
   QMutexLocker lock(&mMutex);

   const auto logWriter = mModuleDest.value(module, nullptr);

   if (logWriter && !logWriter->isStop())
   {
      const auto values = mNonWriterQueue.values(module);

      for (const auto &values : values)
      {
         const auto level = qvariant_cast<LogLevel>(values.at(2).toInt());

         if (logWriter->getLevel() <= level)
         {
            const auto datetime = values.at(0).toDateTime();
            const auto threadId = values.at(1).toString();
            const auto function = values.at(3).toString();
            const auto file = values.at(4).toString();
            const auto line = values.at(5).toInt();
            const auto message = values.at(6).toString();

            logWriter->enqueue(datetime, threadId, module, level, function, file, line, message);
         }
      }

      mNonWriterQueue.remove(module);
   }
}

void QLoggerManager::enqueueMessage(const QString &module, LogLevel level, const QString &message,
                                    const QString &function, const QString &file, int line)
{
   QMutexLocker lock(&mMutex);
   const auto logWriter = mModuleDest.value(module, nullptr);
   const auto isLogEnabled = logWriter && logWriter->getMode() != LogMode::Disabled && !logWriter->isStop();

   if (isLogEnabled && logWriter->getLevel() <= level)
   {
      const auto threadId = QString("%1").arg((quintptr)QThread::currentThread(), QT_POINTER_SIZE * 2, 16, QChar('0'));
      const auto fileName = file.mid(file.lastIndexOf('/') + 1);

      writeAndDequeueMessages(module);

      logWriter->enqueue(QDateTime::currentDateTime(), threadId, module, level, function, fileName, line, message);
   }
   else if (!logWriter && mNonWriterQueue.count(module) < QUEUE_LIMIT)
   {
      const auto threadId = QString("%1").arg((quintptr)QThread::currentThread(), QT_POINTER_SIZE * 2, 16, QChar('0'));
      const auto fileName = file.mid(file.lastIndexOf('/') + 1);

      mNonWriterQueue.insert(module,
                             { QDateTime::currentDateTime(), threadId, QVariant::fromValue<LogLevel>(level), function,
                               fileName, line, message });
   }
}

void QLoggerManager::pause()
{
   QMutexLocker lock(&mMutex);

   mIsStop = true;

   for (auto &logWriter : mModuleDest)
      logWriter->stop(mIsStop);
}

void QLoggerManager::resume()
{
   QMutexLocker lock(&mMutex);

   mIsStop = false;

   for (auto &logWriter : mModuleDest)
      logWriter->stop(mIsStop);
}

void QLoggerManager::overwriteLogMode(LogMode mode)
{
   QMutexLocker lock(&mMutex);

   setDefaultMode(mode);

   for (auto &logWriter : mModuleDest)
      logWriter->setLogMode(mode);
}

void QLoggerManager::overwriteLogLevel(LogLevel level)
{
   QMutexLocker lock(&mMutex);

   setDefaultLevel(level);

   for (auto &logWriter : mModuleDest)
      logWriter->setLogLevel(level);
}

void QLoggerManager::overwriteMaxFileSize(int maxSize)
{
   QMutexLocker lock(&mMutex);

   setDefaultMaxFileSize(maxSize);

   for (auto &logWriter : mModuleDest)
      logWriter->setMaxFileSize(maxSize);
}

QLoggerManager::~QLoggerManager()
{
   QMutexLocker locker(&mMutex);

   for (const auto &dest : mModuleDest.toStdMap())
      writeAndDequeueMessages(dest.first);

   for (auto dest : qAsConst(mModuleDest))
   {
      dest->closeDestination();
      dest->wait();
   }

   for (auto dest : qAsConst(mModuleDest))
      delete dest;

   mModuleDest.clear();
}

}
