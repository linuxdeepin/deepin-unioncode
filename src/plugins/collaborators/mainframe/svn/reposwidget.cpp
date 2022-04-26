#include "reposwidget.h"
#include "amendswidget.h"
#include "historyview.h"
#include "historylogwidget.h"
#include "historydiffwidget.h"
#include "historydisplaywidget.h"
#include "loggindialog.h"
#include "filesourceview.h"
#include "filemodifyview.h"

#include "common/common.h"

#include <QApplication>
#include <QButtonGroup>
#include <QDir>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QSet>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
class ReposWidgetPrivate
{
    friend class ReposWidget;
    QSplitter *splitter{nullptr};
    //    QTimer fileModifyTimer;
    FileSourceView *fileSrcView{nullptr};
    AmendsWidget *amendsWidget{nullptr};
    HistoryDisplayWidget *historyWidget{nullptr};
    QVBoxLayout *vLayout{nullptr};
    LogginDialog *logginDialog{nullptr};
    QToolBar *controlBar{nullptr};
    QButtonGroup *controlGroup{nullptr};
    QToolButton *refreshButton{nullptr};
    QToolButton *updateButton{nullptr};
    QToolButton *optionButton{nullptr};
    QToolButton *historyButton{nullptr};
    QString reposPath;
    QString name;
    QString passwd;
    bool logginResult = false;
    static QString svnProgram;
    static QString svnLogSplitStr;
    // 第一列表示添加、删除或以其他方式更改的项目：
    enum_def(AmendsState_Col1, QString)
    {
        enum_exp NUL = " "; // 没有修改。
        enum_exp SRC = "?"; // 项目不受版本控制。
        enum_exp ADD = "A"; // 项目计划添加。
        enum_exp DEL = "D"; // 项目计划删除。
        enum_exp MOD = "M"; // 项目已修改。
        enum_exp REP = "R"; // 您的工作副本中的项目已被替换。这意味着该文件被安排删除，然后一个同名的新文件被安排添加到它的位置。
        enum_exp CON = "C"; // 项目的内容（相对于属性）与从存储库接收的更新冲突。
        enum_exp EXT = "X"; // 由于外部定义，项目存在。
        enum_exp IGN = "I"; // 项目被忽略（例如，与 svn:ignore属性）。
        enum_exp LOS = "!"; // 项目丢失（例如，您在没有使用svn的情况下移动或删除了它）。这也表明目录不完整（签出或更新被中断）。
        enum_exp OTH = "~"; // 项目被版本化为一种对象（文件、目录、链接），但已被另一种对象替换。
    };

    // 第二列告诉文件或目录属性的状态：
    enum_def(AmendsState_Col2, QString)
    {
        enum_exp NUL = " "; // 没有修改。
        enum_exp MOD = "M"; // 此项目的属性已被修改。
        enum_exp CON = "C"; // 此项目的属性与从存储库接收的属性更新冲突。
    };

    // 仅当工作副本目录被锁定时才会填充第三列：
    enum_def(AmendsState_Col3, QString)
    {
        enum_exp NUL = " "; // 项目未锁定。
        enum_exp LOC = "L"; // 项目已锁定。
    };

    // 仅当项目计划添加历史记录时才会填充第四列：
    enum_def(AmendsState_Col4, QString)
    {
        enum_exp NUL = " "; // 没有安排提交的历史记录。
        enum_exp LOC = "+"; // 与提交一起安排的历史记录。
    };

    // 仅当项相对于其父项切换时才填充第五列（请参阅名为“遍历分支”的部分）：
    enum_def(AmendsState_Col5, QString)
    {
        enum_exp NUL = " "; // Item 是其父目录的子目录。
        enum_exp SWI = "S"; // 项目已切换。
    };

    // 第六列填充了锁定信息：
    enum_def(AmendsState_Col6, QString)
    {
        enum_exp NUL = " "; // 当使用--show-updates ( -u) 时，这意味着文件没有被锁定。如果--show-updates ( -u)没有 被使用，这仅仅意味着文件没有被锁定在这个工作副本中。
        enum_exp KLOC = "K"; // 文件被锁定在这个工作副本中。
        enum_exp OLOC = "O"; // 文件被另一个用户或另一个工作副本锁定。这仅在 使用--show-updates ( -u) 时出现。
        enum_exp TLOC = "T"; // 文件被锁定在此工作副本中，但锁定已“被盗”且无效。该文件当前已锁定在存储库中。这仅在使用--show-updates ( -u) 时出现。
        enum_exp BLOC = "B"; // 文件被锁定在此工作副本中，但锁定已“损坏”且无效。该文件不再被锁定。这仅在使用--show-updates ( -u) 时出现。
    };

    // 仅当项目是树冲突的受害者时才填充第七列：
    enum_def(AmendsState_Col7, QString)
    {
        enum_exp NUL = " "; // 项目不是树冲突的受害者。
        enum_exp CON = "C"; // 项目是树冲突的受害者。
    };

    // 第八列始终为空白。
    enum_def(AmendsState_Col8, QString)
    {
        enum_exp NUL = " ";
    };

    // 过期信息出现在第九列（仅当您通过 --show-updates( -u) 选项时）：
    enum_def(AmendsState_Col9, QString)
    {
        enum_exp NUL = " "; // 您的工作副本中的项目是最新的。
        enum_exp NEW = "*"; // 服务器上存在该项目的较新版本。
    };
};
QString ReposWidgetPrivate::svnProgram{"/usr/bin/svn"};
QString ReposWidgetPrivate::svnLogSplitStr = "------------------------------------"
                                             "------------------------------------\n";
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
    d->splitter->setHandleWidth(2);
    d->fileSrcView = new FileSourceView();
    d->fileSrcView->setMinimumWidth(300);
    d->amendsWidget = new AmendsWidget();
    d->amendsWidget->setMinimumWidth(300);
    d->historyWidget = new HistoryDisplayWidget();
    d->historyWidget->setMinimumWidth(300);

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

    d->fileSrcView->setRootPath(d->reposPath);
}

void ReposWidget::loadRevisionFiles()
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
    }

    d->amendsWidget->modView()->setUpdatesEnabled(false);
    while (process.canReadLine()) {
        QString line = process.readLine();
        QStringList lineList = line.replace("\n", "").split(" ");
        QString filePath = d->reposPath + QDir::separator() + *lineList.rbegin();
        RevisionFile file(*lineList.rbegin(), filePath, lineList.first());
        d->amendsWidget->modView()->addFile(file);
    }
    d->amendsWidget->modView()->setUpdatesEnabled(true);
}

void ReposWidget::reloadRevisionFiles()
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
    }

    d->amendsWidget->modView()->setUpdatesEnabled(false);
    RevisionFiles modFiles;
    while (process.canReadLine()) {
        QString line = process.readLine();
        QStringList lineList = line.replace("\n", "").split(" ");
        QString filePath = d->reposPath + QDir::separator() + *lineList.rbegin();
        RevisionFile file(*lineList.rbegin(), filePath, lineList.first());
        modFiles << file;
    }

    // to changed added file list
    for(int row = 0; row < d->amendsWidget->modView()->rowCount(); row ++) {
        auto rowFile = d->amendsWidget->modView()->file(row); // row revision file
        if (!modFiles.contains(rowFile)) { // 不包含则删除
            d->amendsWidget->modView()->removeFile(rowFile);
        } else { //包含则删除
            modFiles.removeOne(rowFile);
        }
    }
    d->amendsWidget->modView()->addFiles(modFiles);
    d->amendsWidget->modView()->setUpdatesEnabled(true);
}

void ReposWidget::loadHistory()
{
    QProcess process;
    process.setProgram(d->svnProgram);
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"log", "-v"});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return;
    }

    HistoryDatas datas;
    QString line = process.readLine();
    while (process.canReadLine()) {
        if (ReposWidgetPrivate::svnLogSplitStr == line) {
            HistoryData data;
            // header
            line = process.readLine(); // line 1
            if (line.count("|") == 3) {
                auto list = line.replace("\n", "").split(" | ");
                if (list.size() == 4) {
                    data.revision = list[0];
                    data.user = list[1];
                    data.dateTime = list[2];
                    data.lineCount = list[3];
                } else {
                    qCritical() << "Failed, Unkown error header from svn log -v";
                    abort();
                }
            }

            // files
            line = process.readLine(); // line 2
            for (line = process.readLine(); line != "\n"; line = process.readLine()) {
                while (line.startsWith(" "))
                    line = line.remove(0, 1);
                auto list = line.replace("\n", "").split(" ");
                RevisionFile rFile;
                if (list.size() == 2) {
                    rFile.revisionType = list[0];
                    auto fileNameTemp = list[1];
                    if (fileNameTemp.startsWith("/"))
                        fileNameTemp.remove(0, 1);
                    rFile.displayName = fileNameTemp;
                    rFile.filePath = d->reposPath + QDir::separator() + fileNameTemp;
                }
                if (rFile.isValid()) {
                    data.changedFiles << rFile;
                } else {
                    qCritical() << "Failed, Unkown error revision file from svn log -v";
                }
            }

            // desc
            QString descStr;
            for (line = process.readLine();
                 line != "\n" && line != ReposWidgetPrivate::svnLogSplitStr;
                 line = process.readLine()) {
                descStr += line;
            }
            data.description = descStr;
            descStr.clear();
            datas << data;
            for (;line != ReposWidgetPrivate::svnLogSplitStr; line = process.readLine()) {

            }
        }
    }
    d->historyWidget->logWidget()->historyView()->setDatas(datas);
}

void ReposWidget::reloadHistory()
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
        return;
    }
}

void ReposWidget::modFileMenu(const RevisionFile &file, const QPoint &pos)
{
    QMenu menu;
    if (file.revisionType == ReposWidgetPrivate::AmendsState_Col1::get()->ADD) {
        QAction *action = menu.addAction("revert");
        QObject::connect(action, &QAction::triggered, [=](){
            this->revert(file.displayName);
        });
    }
    if (file.revisionType == ReposWidgetPrivate::AmendsState_Col1::get()->SRC) {
        QAction *action = menu.addAction("add");
        QObject::connect(action, &QAction::triggered, [=](){
            this->add(file.displayName);
        });
    }
    menu.exec(pos);
}

void ReposWidget::srcFileMenu(const QString &file, const QPoint &pos)
{

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
    d->vLayout->addWidget(initControlBar(), 0, Qt::AlignHCenter);
    d->vLayout->addWidget(d->splitter);
    d->splitter->addWidget(d->fileSrcView);
    d->splitter->setCollapsible(0, false);
    d->splitter->addWidget(d->amendsWidget);
    d->splitter->setCollapsible(1, false);
    d->splitter->addWidget(d->historyWidget);
    d->splitter->setCollapsible(2, false);

    // 右键菜单
    QObject::connect(d->fileSrcView,
                     &FileSourceView::menuRequest,
                     this, &ReposWidget::srcFileMenu);

    QObject::connect(d->amendsWidget->modView(),
                     &FileModifyView::menuRequest,
                     this, &ReposWidget::modFileMenu);

    loadRevisionFiles(); // 创建修订文件
    loadHistory(); // 添加提交历史

    // 设置文件自动刷新
    //    d->fileModifyTimer.setInterval(500);
    //    QObject::connect(&d->fileModifyTimer, &QTimer::timeout,
    //                     this, &ReposWidget::reloadRevisionFiles);
    //    d->fileModifyTimer.start();

    // 提交信息操作
    QObject::connect(d->amendsWidget, &AmendsWidget::commitClicked,
                     this, &ReposWidget::doAmendsCommit);

    QObject::connect(d->amendsWidget, &AmendsWidget::revertAllClicked,
                     this, &ReposWidget::doAmendsRevertAll);

    // 删除登录界面
    if (d->logginDialog)
        delete d->logginDialog;
}

void ReposWidget::doUpdateRepos()
{
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

void ReposWidget::doRefresh()
{

}

void ReposWidget::doAmendsCommit()
{
    QProcess processCommit;
    processCommit.setWorkingDirectory(d->reposPath);
    processCommit.setProgram(ReposWidgetPrivate::svnProgram);
    QString commitDesc = d->amendsWidget->description();
    processCommit.setArguments({"commit", "-m", commitDesc});
    processCommit.start();
    processCommit.waitForFinished();
    if (processCommit.exitCode() != 0 || processCommit.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(processCommit.readAllStandardError());
    }
}

void ReposWidget::doAmendsRevertAll()
{

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

QWidget *ReposWidget::initControlBar()
{
    static int barHeight = 48;
    static int buttonWidth = 40;
    static int buttonHeight = 40;
    d->controlBar = new QToolBar();
    d->controlBar->setFixedHeight(barHeight);
    d->controlBar->setOrientation(Qt::Orientation::Horizontal);
    d->controlBar->setIconSize({buttonWidth, buttonHeight});

    d->updateButton = new QToolButton();
    d->updateButton->setFixedSize(buttonWidth, buttonHeight);
    d->updateButton->setIcon(QIcon(":/icons/git_pull"));
    d->updateButton->setToolTip(QToolButton::tr("update local from remote repos"));
    QObject::connect(d->updateButton, &QToolButton::toggled, this, &ReposWidget::doUpdateRepos);
    d->controlBar->addWidget(d->updateButton);

    d->refreshButton = new QToolButton();
    d->refreshButton->setFixedSize(buttonWidth, buttonHeight);
    d->refreshButton->setIcon(QIcon(":/icons/refresh"));
    d->refreshButton->setToolTip(QToolButton::tr("refresh current local to display"));
    QObject::connect(d->refreshButton, &QToolButton::toggled, this, &ReposWidget::doRefresh);
    d->controlBar->addWidget(d->refreshButton);
    d->controlBar->addSeparator();

    d->optionButton = new QToolButton();
    d->optionButton->setFixedSize(buttonWidth, buttonHeight);
    d->optionButton->setIcon(QIcon(":/icons/blame"));
    d->optionButton->setToolTip(QToolButton::tr("show repos operation"));
    d->optionButton->setCheckable(true);
    d->controlBar->addWidget(d->optionButton);

    d->historyButton = new QToolButton();
    d->historyButton->setFixedSize(buttonWidth, buttonHeight);
    d->historyButton->setIcon(QIcon(":/icons/git_orange"));
    d->historyButton->setToolTip(QToolButton::tr("show repos history"));
    d->historyButton->setCheckable(true);
    d->controlBar->addWidget(d->historyButton);

    d->controlGroup = new QButtonGroup(d->controlBar);
    d->controlGroup->addButton(d->optionButton);
    d->controlGroup->addButton(d->historyButton);

    QObject::connect(d->controlGroup, QOverload<QAbstractButton *, bool>
                     ::of(&QButtonGroup::buttonToggled),
                     [=](QAbstractButton *button, bool checked){
        if (button == d->optionButton) {
            if (checked) {
                d->fileSrcView->show();
                d->amendsWidget->show();
            } else {
                d->fileSrcView->hide();
                d->amendsWidget->hide();
            }
        }
        if (button == d->historyButton) {
            if (checked) {
                d->historyWidget->show();
            } else {
                d->historyWidget->hide();
            }
        }
    });

    d->fileSrcView->hide();
    d->amendsWidget->hide();
    d->historyWidget->hide();

    d->historyButton->setChecked(true);

    return d->controlBar;
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
