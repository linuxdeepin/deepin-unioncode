#pragma once

#include <QFrame>
#include <JenkinsJobInfo.h>

namespace Jenkins
{

class JobButton : public QFrame
{
   Q_OBJECT

signals:
   void clicked();

public:
   explicit JobButton(const JenkinsJobInfo &job, QWidget *parent = nullptr);

   JenkinsJobInfo getJenkinsJob() const { return mJob; }

protected:
   /**
    * @brief Detects the press event to prepare the click signal.
    *
    * @param e The event
    */
   void mousePressEvent(QMouseEvent *e) override;
   /**
    * @brief Detects the release event and if the press was detected before, it triggers the clicked signal.
    *
    * @param e The event
    */
   void mouseReleaseEvent(QMouseEvent *e) override;

private:
   bool mPressed = false;
   JenkinsJobInfo mJob;
};
}
