#include "reposwidget.h"
#include "amendswidget.h"
#include "commithistorywidget.h"
#include "loggindialog.h"
#include "filemodifywidget.h"

#include "common/common.h"

#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QSet>
#include <QSplitter>
class ReposWidgetPrivate
{
    friend class ReposWidget;
    QSplitter *splitter{nullptr};
    QTimer fileModifyTimer;
    AmendsWidget *amendsWidget{nullptr};
    CommitHistoryWidget *commitHisWidget{nullptr};
    QVBoxLayout *vLayout{nullptr};
    LogginDialog *logginDialog{nullptr};
    QString reposPath;
    QString name;
    QString passwd;
    bool logginResult = false;
    static QString svnProgram;

    enum_def(AmendsFileState, QString)
    {
        enum_exp SRC = "?";
        enum_exp ADD = "A";
    };
};
QString ReposWidgetPrivate::svnProgram{"/usr/bin/svn"};

ReposWidget::ReposWidget(QWidget *parent)
    : QWidget(parent)
    , d(new ReposWidgetPrivate)
{
    d->vLayout = new QVBoxLayout();
    d->logginDialog = new LogginDialog();
    d->vLayout->addWidget(d->logginDialog);
    d->vLayout->setAlignment(d->logginDialog, Qt::AlignCenter);
    setLayout(d->vLayout);

    d->splitter = new QSplitter(Qt::Horizontal);
    d->amendsWidget = new AmendsWidget();
    d->commitHisWidget = new CommitHistoryWidget();
    QObject::connect(d->logginDialog, &LogginDialog::logginOk, this, &ReposWidget::doLoggin);
}

ReposWidget::~ReposWidget()
{
    if (d) {
        delete d;
    }
}

QString ReposWidget::getReposPath() const
{
    return d->reposPath;
}

void ReposWidget::setReposPath(const QString &path)
{
    d->reposPath = path;
    d->logginDialog->setTitleText(LogginDialog::tr("loggin user from svn\nrepos path: %0")
                                  .arg(d->reposPath));
}

bool ReposWidget::loadRevisionFiles()
{
    QProcess process;
    process.setProgram(d->svnProgram);
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"status"});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return false;
    }

    d->amendsWidget->fileSourceWidget()->setUpdatesEnabled(false);
    d->amendsWidget->fileAddedWidget()->setUpdatesEnabled(false);
    while (process.canReadLine()) {
        QString line = process.readLine();
        QStringList lineList = line.replace("\n", "").split(" ");
        QString filePath = d->reposPath + QDir::separator() + *lineList.rbegin();
        if (lineList.first() == ReposWidgetPrivate::AmendsFileState::get()->SRC) {
            RevisionFile file(*lineList.rbegin(), filePath, lineList.first());
            d->amendsWidget->fileSourceWidget()->addFile(file);
        } else if (lineList.first() == ReposWidgetPrivate::AmendsFileState::get()->ADD) {
            RevisionFile file(*lineList.rbegin(), filePath, lineList.first());
            d->amendsWidget->fileAddedWidget()->addFile(file);
        }
    }
    d->amendsWidget->fileSourceWidget()->setUpdatesEnabled(true);
    d->amendsWidget->fileAddedWidget()->setUpdatesEnabled(true);

    return true;
}

bool ReposWidget::reloadRevisionFiles()
{
    QProcess process;
    process.setProgram(d->svnProgram);
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"status"});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return false;
    }

    d->amendsWidget->fileSourceWidget()->setUpdatesEnabled(false);
    d->amendsWidget->fileAddedWidget()->setUpdatesEnabled(false);

    RevisionFiles srcFiles;
    RevisionFiles addFiles;
    while (process.canReadLine()) {
        QString line = process.readLine();
        QStringList lineList = line.replace("\n", "").split(" ");
        QString filePath = d->reposPath + QDir::separator() + *lineList.rbegin();
        RevisionFile file(*lineList.rbegin(), filePath, lineList.first());
        if (lineList.first() == ReposWidgetPrivate::AmendsFileState::get()->SRC) {
            srcFiles << file;
        } else if (lineList.first() == ReposWidgetPrivate::AmendsFileState::get()->ADD) {
            addFiles << file;
        }

    }

    // to changed source file list
    for(int row = 0; row < d->amendsWidget->fileSourceWidget()->count(); row ++) {
        auto rowFile = d->amendsWidget->fileSourceWidget()->file(row); // row revision file
        if (!srcFiles.contains(rowFile)) { // 不包含则删除
            d->amendsWidget->fileSourceWidget()->removeFile(rowFile);
        } else { //包含则删除
            srcFiles.removeOne(rowFile);
        }
    }
    d->amendsWidget->fileSourceWidget()->addFiles(srcFiles);

    // to changed added file list
    for(int row = 0; row < d->amendsWidget->fileAddedWidget()->count(); row ++) {
        auto rowFile = d->amendsWidget->fileAddedWidget()->file(row); // row revision file
        if (!addFiles.contains(rowFile)) { // 不包含则删除
            d->amendsWidget->fileAddedWidget()->removeFile(rowFile);
        } else { //包含则删除
            addFiles.removeOne(rowFile);
        }
    }
    d->amendsWidget->fileAddedWidget()->addFiles(addFiles);

    d->amendsWidget->fileSourceWidget()->setUpdatesEnabled(true);
    d->amendsWidget->fileAddedWidget()->setUpdatesEnabled(true);

    return true;
}

void ReposWidget::doLoggin()
{
    // 设置用户名密码缓存
    setName(d->logginDialog->name());
    setPasswd(d->logginDialog->passwd());

    // 获取登录结果
    d->logginResult = testUserLoggin(d->reposPath, d->name, d->passwd);

    // 无法登录 直接中断流程
    if (!d->logginResult) {
        return;
    }

    // 设置界面
    d->vLayout->addWidget(d->splitter);
    d->splitter->addWidget(d->amendsWidget);
    d->splitter->addWidget(d->commitHisWidget);

    // 右键菜单
    QObject::connect(d->amendsWidget->fileSourceWidget(),
                     &FileModifyWidget::fileMenuRequest,
                     this, &ReposWidget::amendsFileMenu);

    QObject::connect(d->amendsWidget->fileAddedWidget(),
                     &FileModifyWidget::fileMenuRequest,
                     this, &ReposWidget::amendsFileMenu);


    d->amendsWidget->reflashAmends(); // 添加其他界面组件
    loadRevisionFiles(); // 创建修订文件

    // 设置文件自动刷新
    d->fileModifyTimer.setInterval(100);
    QObject::connect(&d->fileModifyTimer, &QTimer::timeout,
                     this, &ReposWidget::reloadRevisionFiles);
    d->fileModifyTimer.start();

    // 提交信息操作
    QObject::connect(d->amendsWidget, &AmendsWidget::commitClicked,
                     this, &ReposWidget::amendsCommit);

    QObject::connect(d->amendsWidget, &AmendsWidget::revertAllClicked,
                     this, &ReposWidget::amendsRevertAll);

    // 删除登录界面
    if (d->logginDialog)
        delete d->logginDialog;
}

void ReposWidget::amendsFileMenu(const RevisionFile &file, const QPoint &pos)
{
    QMenu menu;
    if (file.revisionType == ReposWidgetPrivate::AmendsFileState::get()->ADD) {
        QAction *action = menu.addAction("revert");
        QObject::connect(action, &QAction::triggered, [=](){
            this->revert(file.displayName);
        });
    }
    if (file.revisionType == ReposWidgetPrivate::AmendsFileState::get()->SRC) {
        QAction *action = menu.addAction("add");
        QObject::connect(action, &QAction::triggered, [=](){
            this->add(file.displayName);
        });
    }
    menu.exec(pos);
}

void ReposWidget::amendsCommit()
{
    QProcess processCommit;
    processCommit.setWorkingDirectory(d->reposPath);
    processCommit.setProgram(ReposWidgetPrivate::svnProgram);
    QString commitDesc = d->amendsWidget->amendValue(Description);
    processCommit.setArguments({"commit", "-m", commitDesc});
    processCommit.start();
    processCommit.waitForFinished();
    if (processCommit.exitCode() != 0 || processCommit.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(processCommit.readAllStandardError());
    }

    QProcess processUpdate;
    processUpdate.setWorkingDirectory(d->reposPath);
    processUpdate.setProgram(ReposWidgetPrivate::svnProgram);
    processUpdate.setArguments({"update"});
    processUpdate.start();
    processUpdate.waitForFinished();
    if (processUpdate.exitCode() != 0 || processUpdate.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(processUpdate.readAllStandardError());
    }

}

void ReposWidget::amendsRevertAll()
{
    QProcess process;
    process.setWorkingDirectory(d->reposPath);
    process.setProgram(ReposWidgetPrivate::svnProgram);
    process.setArguments({"revert", "-R", "./"});
    process.start();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
    }
}

bool ReposWidget::testUserLoggin(const QString &reposPath, const QString &name, const QString &passwd)
{
    QProcess process;
    process.setProgram(ReposWidgetPrivate::svnProgram);
    process.setArguments({"list", reposPath, "--username", name, "--password", passwd});
    process.start();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return false;
    }
    return true;
}

bool ReposWidget::add(const QString &display)
{
    QProcess process;
    process.setProgram(d->svnProgram);
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"add", display});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return false;
    }
    return true;
}

bool ReposWidget::revert(const QString &display)
{
    QProcess process;
    process.setProgram(d->svnProgram);
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"revert", display});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return false;
    }
    return true;
}

QString ReposWidget::getName() const
{
    return d->name;
}

void ReposWidget::setName(const QString &value)
{
    d->name = value;
}

QString ReposWidget::getPasswd() const
{
    return d->passwd;
}

void ReposWidget::setPasswd(const QString &value)
{
    d->passwd = value;
}

void ReposWidget::setLogginDisplay(const QString &name, const QString &passwd)
{
    d->logginDialog->setName(name);
    d->logginDialog->setPasswd(passwd);
}
