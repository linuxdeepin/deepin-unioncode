#include <JenkinsJobPanel.h>

#include <BuildGeneralInfoFetcher.h>
#include <CheckBox.h>
#include <ButtonLink.hpp>
#include <QPinnableTabWidget.h>
#include <JobDetailsFetcher.h>
#include <DiffHelper.h>

#include <QLogger.h>

#include <QTimer>
#include <QAuthenticator>
#include <QUrlQuery>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QUrl>
#include <QFile>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QRadioButton>
#include <QButtonGroup>
#include <QButtonGroup>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QMessageBox>

using namespace QLogger;

namespace Jenkins
{

JenkinsJobPanel::JenkinsJobPanel(const IFetcher::Config &config, QWidget *parent)
   : QFrame(parent)
   , mConfig(config)
   , mName(new ButtonLink())
   , mUrl(new ButtonLink(tr("Open job in Jenkins...")))
   , mBuild(new QPushButton(tr("Trigger build")))
   , mManager(new QNetworkAccessManager(this))
{
   setObjectName("JenkinsJobPanel");

   mName->setObjectName("JenkinsJobPanelTitle");

   mScrollFrame = new QFrame();
   mScrollFrame->setObjectName("TransparentScrollArea");

   mLastBuildFrame = new QFrame();
   const auto lastBuildScrollArea = new QScrollArea();
   lastBuildScrollArea->setWidget(mLastBuildFrame);
   lastBuildScrollArea->setWidgetResizable(true);
   lastBuildScrollArea->setFixedHeight(140);
   lastBuildScrollArea->setStyleSheet("background: #404142;");

   const auto scrollArea = new QScrollArea();
   scrollArea->setWidget(mScrollFrame);
   scrollArea->setWidgetResizable(true);
   scrollArea->setStyleSheet("background: #404142;");

   mTabWidget = new QPinnableTabWidget();
   mTabWidget->addPinnedTab(scrollArea, "Previous builds");
   mTabWidget->setContextMenuPolicy(Qt::NoContextMenu);

   mBuild->setVisible(false);
   mBuild->setObjectName("applyActionBtn");

   const auto linkLayout = new QHBoxLayout();
   linkLayout->setContentsMargins(QMargins());
   linkLayout->setSpacing(0);
   linkLayout->addWidget(mUrl);
   linkLayout->addStretch();
   linkLayout->addWidget(mBuild);

   const auto layout = new QVBoxLayout(this);
   layout->setContentsMargins(QMargins());
   layout->setSpacing(10);
   layout->addWidget(mName);
   layout->addLayout(linkLayout);
   layout->addWidget(lastBuildScrollArea);
   layout->addWidget(mTabWidget);

   connect(mName, &ButtonLink::clicked, this, [this]() {
      if (mRequestedJob.name.startsWith("PR-"))
      {
         const auto num = mRequestedJob.name.split("-").last().toInt();
         emit gotoPullRequest(num);
      }
      else
         emit gotoBranch(mRequestedJob.name);
   });
   connect(mUrl, &ButtonLink::clicked, this, [this]() { QDesktopServices::openUrl(mRequestedJob.url); });
   connect(mBuild, &QPushButton::clicked, this, &JenkinsJobPanel::triggerBuild);
}

void JenkinsJobPanel::loadJobInfo(const JenkinsJobInfo &job)
{
   if (mTmpBuildsCounter != 0 && job == mRequestedJob)
   {
      QMessageBox::warning(
          this, tr("Request in progress"),
          tr("There is a request in progress. Please, wait until the builds and stages for this job have been loaded"));
   }
   else
   {
      for (const auto &widget : qAsConst(mTempWidgets))
         delete widget;

      mTempWidgets.clear();

      delete mBuildListLayout;
      mBuildListLayout = nullptr;

      delete mLastBuildLayout;
      mLastBuildLayout = nullptr;

      mTabWidget->setCurrentIndex(0);
      mTabBuildMap.clear();

      for (auto i = 1; i < mTabWidget->count(); ++i)
         mTabWidget->removeTab(i);

      mRequestedJob = job;
      mName->setText(mRequestedJob.name);

      mTmpBuildsCounter = mRequestedJob.builds.count();

      if (mRequestedJob.builds.isEmpty())
         mBuild->setVisible(true);
      else
      {
         mBuild->setVisible(false);
         for (const auto &build : qAsConst(mRequestedJob.builds))
         {
            const auto buildFetcher = new BuildGeneralInfoFetcher(mConfig, build, this);
            connect(buildFetcher, &BuildGeneralInfoFetcher::signalBuildInfoReceived, this,
                    [this](const JenkinsJobBuildInfo &build) { appendJobsData(mRequestedJob.name, build); });
            connect(buildFetcher, &BuildGeneralInfoFetcher::signalBuildInfoReceived, buildFetcher,
                    &BuildGeneralInfoFetcher::deleteLater);

            buildFetcher->triggerFetch();
         }
      }
   }
}

void JenkinsJobPanel::appendJobsData(const QString &jobName, const JenkinsJobBuildInfo &build)
{
   if (jobName != mRequestedJob.name)
      return;

   auto iter = std::find(mRequestedJob.builds.begin(), mRequestedJob.builds.end(), build);

   if (iter == mRequestedJob.builds.end())
      mRequestedJob.builds.append(build);
   else
      *iter = build;

   --mTmpBuildsCounter;

   if (mTmpBuildsCounter == 0)
   {
      mBuildListLayout = new QVBoxLayout(mScrollFrame);
      mBuildListLayout->setContentsMargins(QMargins());
      mBuildListLayout->setSpacing(10);

      mLastBuildLayout = new QHBoxLayout(mLastBuildFrame);
      mLastBuildLayout->setContentsMargins(10, 10, 10, 10);
      mLastBuildLayout->setSpacing(10);

      std::sort(mRequestedJob.builds.begin(), mRequestedJob.builds.end(),
                [](const JenkinsJobBuildInfo &build1, const JenkinsJobBuildInfo &build2) {
                   return build1.number > build2.number;
                });

      const auto build = mRequestedJob.builds.takeFirst();

      fillBuildLayout(build, mLastBuildLayout);

      for (const auto &build : qAsConst(mRequestedJob.builds))
      {
         const auto stagesLayout = new QHBoxLayout();
         stagesLayout->setContentsMargins(10, 10, 10, 10);
         stagesLayout->setSpacing(10);

         fillBuildLayout(build, stagesLayout);

         mBuildListLayout->addLayout(stagesLayout);
      }

      const auto hasCustomBuildConfig = !mRequestedJob.configFields.isEmpty();

      mBuild->setVisible(!hasCustomBuildConfig);

      if (hasCustomBuildConfig)
         createBuildConfigPanel();

      mBuildListLayout->addStretch();
   }
}

void JenkinsJobPanel::fillBuildLayout(const JenkinsJobBuildInfo &build, QHBoxLayout *layout)
{
   const auto mark = new ButtonLink(QString::number(build.number));
   mark->setToolTip(build.result);
   mark->setFixedSize(30, 30);
   mark->setStyleSheet(QString("QLabel{"
                               "background: %1;"
                               "border-radius: 15px;"
                               "qproperty-alignment: AlignCenter;"
                               "}")
                           .arg(Jenkins::resultColor(build.result).name()));
   connect(mark, &ButtonLink::clicked, this, [this, build]() {
      showArtifacts(build);
      requestFile(build);
   });

   mTempWidgets.append(mark);

   layout->addWidget(mark);

   for (const auto &stage : build.stages)
   {
      QTime t = QTime(0, 0, 0).addMSecs(stage.duration);
      const auto tStr = t.toString("HH:mm:ss.zzz");
      const auto label = new QLabel(QString("%1").arg(stage.name));
      label->setToolTip(tStr);
      label->setObjectName("StageStatus");
      label->setFixedSize(100, 80);
      label->setWordWrap(true);
      label->setStyleSheet(QString("QLabel{"
                                   "background: %1;"
                                   "color: white;"
                                   "border-radius: 10px;"
                                   "border-bottom-right-radius: 0px;"
                                   "border-bottom-left-radius: 0px;"
                                   "qproperty-alignment: AlignCenter;"
                                   "padding: 5px;"
                                   "}")
                               .arg(Jenkins::resultColor(stage.result).name()));
      mTempWidgets.append(label);

      const auto time = new QLabel(tStr);
      time->setToolTip(tStr);
      time->setFixedSize(100, 20);
      time->setStyleSheet(QString("QLabel{"
                                  "background: %1;"
                                  "color: white;"
                                  "border-radius: 10px;"
                                  "border-top-right-radius: 0px;"
                                  "border-top-left-radius: 0px;"
                                  "qproperty-alignment: AlignCenter;"
                                  "padding: 5px;"
                                  "}")
                              .arg(Jenkins::resultColor(stage.result).name()));
      mTempWidgets.append(time);

      const auto stageLayout = new QVBoxLayout();
      stageLayout->setContentsMargins(QMargins());
      stageLayout->setSpacing(0);
      stageLayout->addWidget(label);
      stageLayout->addWidget(time);
      stageLayout->addStretch();

      layout->addLayout(stageLayout);
   }

   layout->addStretch();
}

void JenkinsJobPanel::requestFile(const JenkinsJobBuildInfo &build)
{
   if (mTabBuildMap.contains(build.number))
      mTabWidget->setCurrentIndex(mTabBuildMap.value(build.number));
   else
   {
      auto urlStr = build.url;
      urlStr.append("/consoleText");
      QNetworkRequest request(urlStr);

      if (!mConfig.user.isEmpty() && !mConfig.token.isEmpty())
         request.setRawHeader(QByteArray("Authorization"),
                              QString("Basic %1:%2").arg(mConfig.user, mConfig.token).toLocal8Bit().toBase64());

      const auto reply = mManager->get(request);
      connect(reply, &QNetworkReply::finished, this, [this, number = build.number]() { storeFile(number); });
   }
}

void JenkinsJobPanel::storeFile(int buildNumber)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   const auto data = reply->readAll();

   if (!data.isEmpty())
   {
      const auto text = new QPlainTextEdit(QString::fromUtf8(data));
      text->setReadOnly(true);
      text->setObjectName("JenkinsOutput");
      mTempWidgets.append(text);

      const auto find = new QLineEdit();
      find->setPlaceholderText(tr("Find text... "));
      connect(find, &QLineEdit::editingFinished, this,
              [this, text, find]() { DiffHelper::findString(find->text(), text, this); });
      mTempWidgets.append(find);

      const auto frame = new QFrame();
      frame->setObjectName("JenkinsOutput");

      const auto layout = new QVBoxLayout(frame);
      layout->setContentsMargins(10, 10, 10, 10);
      layout->setSpacing(10);
      layout->addWidget(find);
      layout->addWidget(text);

      const auto index = mTabWidget->addTab(frame, QString("Output for #%1").arg(buildNumber));
      mTabWidget->setCurrentIndex(index);

      mTabBuildMap.insert(buildNumber, index);
   }

   reply->deleteLater();
}

void JenkinsJobPanel::createBuildConfigPanel()
{
   const auto buildFrame = new QFrame();
   buildFrame->setObjectName("buildFrame");
   buildFrame->setStyleSheet("#buildFrame { background: #404142; }");

   const auto buildLayout = new QGridLayout(buildFrame);
   buildLayout->setContentsMargins(10, 10, 10, 10);
   buildLayout->setSpacing(10);

   auto row = 0;

   for (const auto &config : qAsConst(mRequestedJob.configFields))
   {
      buildLayout->addWidget(new QLabel(config.name), row, 0);

      if (config.fieldType == JobConfigFieldType::Bool)
      {
         const auto check = new CheckBox();
         check->setChecked(config.defaultValue.toBool());
         mBuildValues[config.name] = qMakePair(JobConfigFieldType::Bool, config.defaultValue);
         connect(check, &CheckBox::stateChanged, this, [this, name = config.name](int checkState) {
            mBuildValues[name] = qMakePair(JobConfigFieldType::Bool, checkState == Qt::Checked);
         });

         buildLayout->addWidget(check, row, 1);
      }
      else if (config.fieldType == JobConfigFieldType::String)
      {
         const auto lineEdit = new QLineEdit();
         lineEdit->setText(config.defaultValue.toString());
         mBuildValues[config.name] = qMakePair(JobConfigFieldType::Bool, config.defaultValue);
         connect(lineEdit, &QLineEdit::textChanged, this, [this, name = config.name](const QString &text) {
            mBuildValues[name] = qMakePair(JobConfigFieldType::String, text);
         });

         buildLayout->addWidget(lineEdit, row, 1);
      }
      else if (config.fieldType == JobConfigFieldType::Choice)
      {
         const auto combo = new QComboBox();
         combo->addItems(config.choicesValues);
         mBuildValues[config.name] = qMakePair(JobConfigFieldType::Bool, config.defaultValue);
         connect(combo, &QComboBox::currentTextChanged, this, [this, name = config.name](const QString &text) {
            mBuildValues[name] = qMakePair(JobConfigFieldType::String, text);
         });

         if (!config.defaultValue.toString().isEmpty())
            combo->setCurrentText(config.defaultValue.toString());

         buildLayout->addWidget(combo, row, 1);
      }

      ++row;
   }

   const auto btnsLayout = new QHBoxLayout();
   btnsLayout->setContentsMargins(QMargins());
   btnsLayout->setSpacing(0);

   const auto btn = new QPushButton(tr("Build"));
   btn->setFixedSize(100, 30);
   btn->setObjectName("applyActionBtn");
   connect(btn, &QPushButton::clicked, this, &JenkinsJobPanel::triggerBuild);

   btnsLayout->addWidget(btn);
   btnsLayout->addStretch();

   buildLayout->addLayout(btnsLayout, row, 0, 1, 2);
   buildLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), row, 3);

   mTabWidget->addPinnedTab(buildFrame, tr("Build with params"));
}

void JenkinsJobPanel::triggerBuild()
{
   const auto endpoint = QString::fromUtf8(mRequestedJob.builds.isEmpty() ? "build" : "buildWithParameters");
   QUrl url(mRequestedJob.url.endsWith("/") ? QString("%1%2").arg(mRequestedJob.url, endpoint)
                                            : QString("%1/%2").arg(mRequestedJob.url, endpoint));
   QNetworkRequest request(url);

   if (!mConfig.user.isEmpty() && !mConfig.token.isEmpty())
   {
      const auto data = QString("%1:%2").arg(mConfig.user, mConfig.token).toLocal8Bit().toBase64();
      request.setRawHeader("Authorization", QString(QString::fromUtf8("Basic ") + data).toLocal8Bit());
   }

   QUrlQuery query;

   const auto end = mBuildValues.constEnd();
   for (auto iter = mBuildValues.constBegin(); iter != end; ++iter)
   {
      const auto value = iter->second.toString();
      if (!value.isEmpty())
         query.addQueryItem(iter.key(), value);
   }

   const auto queryData = query.query().toUtf8();
   mManager->post(request, queryData);

   QTimer::singleShot(10000, this, [this]() {
      const auto jobRequest = new JobDetailsFetcher(mConfig, mRequestedJob);
      connect(jobRequest, &JobDetailsFetcher::signalJobDetailsRecieved, this, [this](const JenkinsJobInfo &newInfo) {
         mRequestedJob.builds = newInfo.builds;
         mRequestedJob.configFields = newInfo.configFields;
         mRequestedJob.healthStatus = newInfo.healthStatus;

         loadJobInfo(mRequestedJob);
      });
      connect(jobRequest, &JobDetailsFetcher::signalJobDetailsRecieved, jobRequest, &JobDetailsFetcher::deleteLater);

      jobRequest->triggerFetch();
   });

   mBuild->setVisible(false);

   QMessageBox::information(this, tr("Update requested"),
                            tr("The build has been triggered and the information will be refreshed in 10 secs."));
}

void JenkinsJobPanel::showArtifacts(const JenkinsJobBuildInfo &build)
{
   const auto artifactsLayout = new QVBoxLayout();
   artifactsLayout->setContentsMargins(QMargins());
   artifactsLayout->setSpacing(10);

   const auto artifactsFrame = new QFrame();
   artifactsFrame->setLayout(artifactsLayout);
   artifactsFrame->setObjectName("artifactsFrame");
   artifactsFrame->setStyleSheet("#artifactsFrame{ background: #404142; }");

   const auto scroll = new QScrollArea();
   scroll->setWidget(artifactsFrame);
   scroll->setWidgetResizable(true);
   scroll->setObjectName("artifactsFrame");
   scroll->setStyleSheet("#artifactsFrame{ background: #404142; }");

   for (const auto &artifact : build.artifacts)
   {
      const auto fileLink = new ButtonLink(artifact.fileName);
      connect(fileLink, &ButtonLink::clicked, this,
              [this, artifact, num = build.number]() { downloadArtifact(artifact, num); });
      artifactsLayout->addWidget(fileLink);
   }

   mTabWidget->addTab(scroll, tr("Artifacts for #%1").arg(build.number));
}

void JenkinsJobPanel::downloadArtifact(const JenkinsJobBuildInfo::Artifact &artifact, int number)
{
   QNetworkRequest request(artifact.url);

   if (!mConfig.user.isEmpty() && !mConfig.token.isEmpty())
   {
      const auto data = QString("%1:%2").arg(mConfig.user, mConfig.token).toLocal8Bit().toBase64();
      request.setRawHeader("Authorization", QString(QString::fromUtf8("Basic ") + data).toLocal8Bit());
   }

   const auto reply = mManager->get(request);
   connect(reply, &QNetworkReply::finished, this,
           [this, fileName = artifact.fileName, number] { storeArtifact(fileName, number); });
}

void JenkinsJobPanel::storeArtifact(const QString &fileName, int buildNumber)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   const auto data = reply->readAll();

   if (!data.isEmpty())
   {
      auto fullPath = QString("%1/%2_%3")
                          .arg(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                               QString::number(buildNumber), fileName);
      QFile f(fullPath);

      if (f.exists())
      {
         QMessageBox::warning(this, tr("File already exists!"),
                              tr("The file already exists in %1.")
                                  .arg(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
      }
      else if (f.open(QIODevice::WriteOnly))
      {
         f.write(data);
         f.close();

         QMessageBox::information(
             this, tr("File downloaded!"),
             tr("The file (%1) has been downloaded in: %2")
                 .arg(fileName, QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
      }
   }
   else
      QMessageBox::warning(this, tr("File download error!"), tr("The file (%1) couldn't be downloaded.").arg(fileName));
}

}
