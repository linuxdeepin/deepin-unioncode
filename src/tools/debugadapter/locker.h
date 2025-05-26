// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
