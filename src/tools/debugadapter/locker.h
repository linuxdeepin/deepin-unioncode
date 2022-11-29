/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LOCKER_H
#define LOCKER_H

#include <mutex>
#include <condition_variable>

#include <QMessageBox>
#include <QDebug>

// Event provides a basic wait and signal synchronization primitive.
class ConditionLock {
 public:
  // wait() blocks until the event is fired.
  void wait()
  {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [&] { return fired; });
  }

  // fire() sets signals the event, and unblocks any calls to wait().
  void fire()
  {
      std::unique_lock<std::mutex> lock(mutex);
      fired = true;
      cv.notify_all();
  }

  void reset()
  {
       fired = false;
  }

 private:
  std::mutex mutex;
  std::condition_variable cv;
  bool fired = false;
};

class ConditionLockEx {
 public:
  // wait() blocks until the event is fired.
  void wait(int seconds = 3)
  {
      std::unique_lock<std::mutex> lock(mutex);
      if (cv.wait_for(lock, std::chrono::seconds(seconds)) == std::cv_status::timeout) {
          qCritical() << "!!!Time Out!!!";
      }
  }

  // fire() sets signals the event, and unblocks any calls to wait().
  void fire()
  {
      std::unique_lock<std::mutex> lock(mutex);
      cv.notify_all();
  }

 private:
  std::mutex mutex;
  std::condition_variable cv;
};

#endif // LOCKER_H
