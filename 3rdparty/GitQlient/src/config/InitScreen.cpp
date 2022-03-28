#include "InitScreen.h"

#include <GeneralConfigDlg.h>
#include <CreateRepoDlg.h>
#include <ProgressDlg.h>
#include <GitQlientSettings.h>
#include <GitBase.h>
#include <GitConfig.h>
#include <ButtonLink.hpp>

#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QStyle>
#include <QLabel>
#include <QApplication>
#include <QMessageBox>
#include <QtGlobal>
#include <QDesktopServices>
#include <GitQlientStyles.h>

#include <QLogger.h>

using namespace QLogger;

InitScreen::InitScreen(QWidget *parent)
   : QFrame(parent)
   , mOpenRepo(new QPushButton(tr("OPEN")))
   , mCloneRepo(new QPushButton(tr("CLONE")))
   , mInitRepo(new QPushButton(tr("NEW")))
{
   setAttribute(Qt::WA_DeleteOnClose);

   setStyleSheet(GitQlientStyles::getStyles());

   mOpenRepo->setObjectName("bigButton");
   mCloneRepo->setObjectName("bigButton");
   mInitRepo->setObjectName("bigButton");

   // Adding buttons to open or init repos
   const auto repoOptionsFrame = new QFrame();
   const auto repoOptionsLayout = new QHBoxLayout(repoOptionsFrame);
   repoOptionsLayout->setSpacing(0);
   repoOptionsLayout->setContentsMargins(QMargins());
   repoOptionsLayout->addWidget(mOpenRepo);
   repoOptionsLayout->addStretch();
   repoOptionsLayout->addWidget(mCloneRepo);
   repoOptionsLayout->addStretch();
   repoOptionsLayout->addWidget(mInitRepo);

   const auto projectsFrame = new QFrame();
   mRecentProjectsLayout = new QVBoxLayout(projectsFrame);
   mRecentProjectsLayout->setContentsMargins(QMargins());
   mRecentProjectsLayout->addWidget(createRecentProjectsPage());

   const auto mostUsedProjectsFrame = new QFrame();
   mUsedProjectsLayout = new QVBoxLayout(mostUsedProjectsFrame);
   mUsedProjectsLayout->setContentsMargins(QMargins());
   mUsedProjectsLayout->addWidget(createUsedProjectsPage());

   const auto widgetsLayout = new QHBoxLayout();
   widgetsLayout->setSpacing(10);
   widgetsLayout->setContentsMargins(0, 10, 0, 10);
   widgetsLayout->addWidget(projectsFrame);
   widgetsLayout->addStretch();
   widgetsLayout->addWidget(mostUsedProjectsFrame);

   const auto title = new QLabel(tr("GitQlient %1").arg(VER));
   title->setObjectName("title");

   const auto gitqlientIcon = new QLabel();
   QIcon stagedIcon(":/icons/GitQlientLogoSVG");
   gitqlientIcon->setPixmap(stagedIcon.pixmap(96, 96));

   const auto configBtn = new QPushButton();
   configBtn->setIcon(QIcon(":/icons/config"));
   connect(configBtn, &QPushButton::clicked, this, &InitScreen::openConfigDlg);

   const auto titleLayout = new QHBoxLayout();
   titleLayout->setContentsMargins(QMargins());
   titleLayout->setSpacing(10);
   titleLayout->addStretch();
   titleLayout->addWidget(gitqlientIcon);
   titleLayout->addWidget(title);
   titleLayout->addStretch();
   titleLayout->addWidget(configBtn);

   const auto lineTitle = new QFrame();
   lineTitle->setObjectName("orangeHSeparator");

   const auto lineTitle2 = new QFrame();
   lineTitle2->setObjectName("orangeHSeparator");

   const auto version = new ButtonLink(tr("About GitQlient..."));
   connect(version, &ButtonLink::clicked, this, &InitScreen::showAbout);
   version->setToolTip(QString("%1").arg(SHA_VER));

   const auto goToRepo = new ButtonLink(tr("Source code"));
   connect(goToRepo, &ButtonLink::clicked, this,
           []() { QDesktopServices::openUrl(QUrl("https://www.github.com/francescmm/GitQlient")); });
   goToRepo->setToolTip(tr("Get the source code in GitHub"));

   const auto goToBlog = new ButtonLink(tr("Report an issue"));
   connect(goToBlog, &ButtonLink::clicked, this,
           []() { QDesktopServices::openUrl(QUrl("https://github.com/francescmm/GitQlient/issues/new/choose")); });
   goToBlog->setToolTip(tr("Report an issue in GitHub"));

   const auto promoLayout = new QHBoxLayout();
   promoLayout->setContentsMargins(QMargins());
   promoLayout->setSpacing(0);
   promoLayout->addWidget(version);
   promoLayout->addStretch();
   promoLayout->addWidget(goToRepo);
   promoLayout->addStretch();
   promoLayout->addWidget(goToBlog);

   const auto centerFrame = new QFrame();
   centerFrame->setObjectName("InitWidget");
   const auto centerLayout = new QVBoxLayout(centerFrame);
   centerLayout->setSpacing(10);
   centerLayout->setContentsMargins(QMargins());
   centerLayout->addLayout(titleLayout);
   centerLayout->addWidget(lineTitle);
   centerLayout->addWidget(repoOptionsFrame);
   centerLayout->addLayout(widgetsLayout);
   centerLayout->addWidget(lineTitle2);
   centerLayout->addLayout(promoLayout);
   centerLayout->addStretch();

   const auto layout = new QGridLayout(this);
   layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);
   layout->addWidget(centerFrame, 1, 1);
   layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 2);

   connect(mOpenRepo, &QPushButton::clicked, this, &InitScreen::openRepo);
   connect(mCloneRepo, &QPushButton::clicked, this, &InitScreen::cloneRepo);
   connect(mInitRepo, &QPushButton::clicked, this, &InitScreen::initRepo);

   const auto gitBase(QSharedPointer<GitBase>::create(""));
   mGit = QSharedPointer<GitConfig>::create(gitBase);

   connect(mGit.data(), &GitConfig::signalCloningProgress, this, &InitScreen::updateProgressDialog,
           Qt::DirectConnection);
   connect(mGit.data(), &GitConfig::signalCloningFailure, this, &InitScreen::showError, Qt::DirectConnection);
}

void InitScreen::openRepo()
{
   const QString dirName(QFileDialog::getExistingDirectory(this, "Choose the directory of a Git project"));

   if (!dirName.isEmpty())
   {
      QDir d(dirName);
      emit signalOpenRepo(d.absolutePath());
   }
}

void InitScreen::cloneRepo()
{
   CreateRepoDlg cloneDlg(CreateRepoDlgType::CLONE, mGit);
   connect(&cloneDlg, &CreateRepoDlg::signalOpenWhenFinish, this, [this](const QString &path) { mPathToOpen = path; });

   if (cloneDlg.exec() == QDialog::Accepted)
   {
      mProgressDlg = new ProgressDlg(tr("Loading repository..."), QString(), 100, false);
      connect(mProgressDlg, &ProgressDlg::destroyed, this, [this]() { mProgressDlg = nullptr; });
      mProgressDlg->show();
   }
}

void InitScreen::initRepo()
{
   CreateRepoDlg cloneDlg(CreateRepoDlgType::INIT, mGit);
   connect(&cloneDlg, &CreateRepoDlg::signalOpenWhenFinish, this, &InitScreen::signalOpenRepo);
   cloneDlg.exec();
}

QWidget *InitScreen::createRecentProjectsPage()
{
   delete mInnerWidget;
   mInnerWidget = new QFrame();

   auto title = new QLabel(tr("Recent"));
   title->setStyleSheet("font-size: 14pt;");

   const auto innerLayout = new QVBoxLayout(mInnerWidget);
   innerLayout->setContentsMargins(QMargins());
   innerLayout->setSpacing(10);
   innerLayout->addWidget(title);

   const auto projects = GitQlientSettings().getRecentProjects();

   for (auto project : projects)
   {
      const auto projectName = project.mid(project.lastIndexOf("/") + 1);
      const auto labelText = QString("%1<br><em>%2</em>").arg(projectName, project);
      const auto clickableFrame = new ButtonLink(labelText, this);
      connect(clickableFrame, &ButtonLink::clicked, this, [this, project]() { emit signalOpenRepo(project); });
      innerLayout->addWidget(clickableFrame);
   }

   innerLayout->addStretch();

   const auto clear = new ButtonLink(tr("Clear list"));
   clear->setVisible(!projects.isEmpty());
   connect(clear, &ButtonLink::clicked, this, [this]() {
      GitQlientSettings().clearRecentProjects();

      mRecentProjectsLayout->addWidget(createRecentProjectsPage());
   });

   const auto lineTitle = new QFrame();
   lineTitle->setObjectName("separator");

   innerLayout->addWidget(lineTitle);
   innerLayout->addWidget(clear);

   return mInnerWidget;
}

QWidget *InitScreen::createUsedProjectsPage()
{
   delete mMostUsedInnerWidget;
   mMostUsedInnerWidget = new QFrame();

   auto title = new QLabel(tr("Most used"));
   title->setStyleSheet("font-size: 14pt;");

   const auto innerLayout = new QVBoxLayout(mMostUsedInnerWidget);
   innerLayout->setContentsMargins(QMargins());
   innerLayout->setSpacing(10);
   innerLayout->addWidget(title);

   const auto projects = GitQlientSettings().getMostUsedProjects();

   for (auto project : projects)
   {
      const auto projectName = project.mid(project.lastIndexOf("/") + 1);
      const auto labelText = QString("%1<br><em>%2</em>").arg(projectName, project);
      const auto clickableFrame = new ButtonLink(labelText);
      connect(clickableFrame, &ButtonLink::clicked, this, [this, project]() { emit signalOpenRepo(project); });
      innerLayout->addWidget(clickableFrame);
   }

   innerLayout->addStretch();

   const auto clear = new ButtonLink(tr("Clear list"));
   clear->setVisible(!projects.isEmpty());
   connect(clear, &ButtonLink::clicked, this, [this]() {
      GitQlientSettings().clearMostUsedProjects();

      mUsedProjectsLayout->addWidget(createUsedProjectsPage());
   });

   const auto lineTitle = new QFrame();
   lineTitle->setObjectName("separator");

   innerLayout->addWidget(lineTitle);
   innerLayout->addWidget(clear);

   return mMostUsedInnerWidget;
}

void InitScreen::updateProgressDialog(QString stepDescription, int value)
{
   if (value >= 0)
   {
      mProgressDlg->setValue(value);

      if (stepDescription.contains("done", Qt::CaseInsensitive))
      {
         mProgressDlg->close();
         emit signalOpenRepo(mPathToOpen);

         mPathToOpen = "";
      }
   }

   mProgressDlg->setLabelText(stepDescription);
   mProgressDlg->repaint();
}

void InitScreen::showError(int, QString description)
{
   if (mProgressDlg)
      mProgressDlg->deleteLater();

   QMessageBox::critical(this, tr("Error!"), description);
}

void InitScreen::showAbout()
{
   const QString aboutMsg = tr(
       "GitQlient, pronounced as git+client (/gɪtˈklaɪənt/) is a multi-platform Git client. "
       "With GitQlient you will be able to add commits, branches and manage all the options Git provides. <br><br>"
       "Once a fork of QGit, GitQlient has followed is own path. "
       "You can download the code from <a href='https://github.com/francescmm/GitQlient'>GitHub</a>. If you find any "
       "bug or problem, please report it in <a href='https://github.com/francescmm/GitQlient/issues'>the issues "
       "page</a> so I can fix it as soon as possible.<br><br>"
       "If you want to integrate GitQlient into QtCreator, I also provide a plugin that you can download from "
       "<a href='https://github.com/francescmm/GitQlient/releases'>here</a>. Just make sure you pick the right "
       "version and follow the instructions in the main page of the repo.<br><br>"
       "GitQlient can be compiled from Qt 5.12 on.<br><br>"
       "Copyright &copy; 2019 - 2020 GitQlient (Francesc Martínez)");

   QMessageBox::about(this, tr("About GitQlient v%1").arg(VER), aboutMsg);
}

void InitScreen::openConfigDlg()
{
   GeneralConfigDlg dlg;
   dlg.exec();
}

void InitScreen::onRepoOpened()
{
   mRecentProjectsLayout->addWidget(createRecentProjectsPage());
   mUsedProjectsLayout->addWidget(createUsedProjectsPage());
}
