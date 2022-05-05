#include "svnclientwidget.h"
#include "reposwidget.h"
#include "checkoutdialog.h"
#include "common/common.h"

#include "QPinnableTabWidget.h"
#include "GitQlientStyles.h"
#include "QLogger.h"

#include <QMenu>
#include <QEvent>
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QLabel>
#include <QProcess>
#include <QFileDialog>

SvnClientWidget::SvnClientWidget(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow (parent, flags)
    , mRepos(new QPinnableTabWidget())
{
    mRepos = new QPinnableTabWidget();
    const auto homeMenu = new QPushButton();
    const auto menu = new QMenu(homeMenu);
    menu->installEventFilter(this);

    homeMenu->setIcon(QIcon(":/icons/burger_menu"));
    homeMenu->setIconSize(QSize(17, 17));
    homeMenu->setToolTip("Options");
    homeMenu->setMenu(menu);
    homeMenu->setObjectName("MainMenuBtn");

    const auto clone = menu->addAction(QAction::tr("Checkout repository"));
    connect(clone, &QAction::triggered, this, &SvnClientWidget::showCheckoutDialog);

    const auto open = menu->addAction(QAction::tr("Open repository"));
    connect(open, &QAction::triggered, this, &SvnClientWidget::showOpenLocalRepos);

    mRepos->setObjectName("GitQlientTab");
    mRepos->setStyleSheet(GitQlientStyles::getStyles());
    mRepos->setCornerWidget(homeMenu, Qt::TopLeftCorner);
    setCentralWidget(mRepos);
    addRepoTab("/home/funning/Documents/svn");
}

void SvnClientWidget::addRepoTab(const QString &repoPath, const QString &user, const QString &passwd)
{
    if (!isSvnDir(repoPath)) {
        ContextDialog::ok(QDialog::tr("Open path failed, current repos not svn subdir"));
        return;
    }
    addNewRepoTab(repoPath, user, passwd);
}

void SvnClientWidget::addNewRepoTab(const QString &repoPathArg, const QString &user, const QString &passwd)
{
    const auto repoPath = QFileInfo(repoPathArg).canonicalFilePath();

    if (!mCurrentRepos.contains(repoPath)) {
        const auto repoName = repoPath.contains("/") ? repoPath.split("/").last() : "";
        auto reposWidget = new ReposWidget;
        reposWidget->setName(user);
        reposWidget->setPasswd(passwd);
        reposWidget->setReposPath(repoPathArg);
        const int index = mRepos->addTab(reposWidget, repoName);
        mRepos->setTabIcon(index, QIcon(QString(":/icons/local")));
    }
}

void SvnClientWidget::showCheckoutDialog()
{
    CheckoutDialog dialog;
    dialog.connect(&dialog, &CheckoutDialog::checkoutRepos, this, &SvnClientWidget::doCheckoutRepos);
    dialog.exec();
}

void SvnClientWidget::showOpenLocalRepos()
{
    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setWindowTitle(QDialog::tr("select local reops"));
    dialog.exec();
    auto urls = dialog.selectedUrls();
    if (!urls.isEmpty()) {
       addRepoTab(urls.first().toLocalFile());
    }
}

void SvnClientWidget::doCheckoutRepos(const QString &remote, const QString &local, const QString &user, const QString &passwd)
{
    QProcess process;
    process.setProgram(svnProgram());
    process.setArguments({"checkout", remote, local, "--username", user, "--password", passwd});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return;
    }

    auto okCb = [=](bool checked){
        Q_UNUSED(checked)
        addRepoTab(local, user, passwd);
    };

    ContextDialog::okCancel(QString("checkout repos successful, now to open with user %0?").arg(user),
                            "Message", QMessageBox::Icon::Question, okCb);
}

bool SvnClientWidget::isSvnDir(const QString &repoPath)
{
    QDir dir(repoPath + QDir::separator() + ".svn");
    return dir.exists();
}

bool SvnClientWidget::eventFilter(QObject *obj, QEvent *event)
{
    const auto menu = qobject_cast<QMenu *>(obj);
    if (menu && event->type() == QEvent::Show) {
        auto localPos = menu->parentWidget()->pos();
        auto pos = mapToGlobal(localPos);
        menu->show();
        pos.setY(pos.y() + menu->parentWidget()->height());
        menu->move(pos);
        return true;
    }

    return false;
}
