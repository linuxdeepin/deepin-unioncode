#include "JobButton.h"

#include <JenkinsJobInfo.h>

#include <QLabel>
#include <QIcon>
#include <QHBoxLayout>
#include <QMouseEvent>

namespace Jenkins
{
JobButton::JobButton(const JenkinsJobInfo &job, QWidget *parent)
   : QFrame(parent)
   , mJob(job)
{
   mJob.name.replace("%2F", "/");
   mJob.color.remove("_anime");

   if (mJob.color.contains("blue"))
      mJob.color = "green";
   else if (mJob.color.contains("disabled") || mJob.color.contains("grey") || mJob.color.contains("notbuilt"))
      mJob.color = "grey";
   else if (mJob.color.contains("aborted"))
      mJob.color = "dark_grey";

   const auto icon = new QLabel();
   icon->setPixmap(QIcon(QString(":/icons/%1").arg(mJob.color)).pixmap(22, 22));

   const auto layout = new QHBoxLayout(this);
   layout->setContentsMargins(QMargins());
   layout->setSpacing(20);
   layout->addWidget(icon);
   layout->addWidget(new QLabel(mJob.name));
   layout->addStretch();
}

void JobButton::mousePressEvent(QMouseEvent *e)
{
   mPressed = rect().contains(e->pos()) && e->button() == Qt::LeftButton;
}

void JobButton::mouseReleaseEvent(QMouseEvent *e)
{
   if (mPressed && rect().contains(e->pos()) && e->button() == Qt::LeftButton)
      emit clicked();
}

}
