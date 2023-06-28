// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reposwidget.h"
#include "amendswidget.h"
#include "historyview.h"
#include "historylogwidget.h"
#include "historydiffwidget.h"
#include "historydisplaywidget.h"
#include "loggindialog.h"
#include "filesourceview.h"
#include "filemodifyview.h"
#include "historydiffview.h"

#include "common/common.h"

#include "FileDiffView.h"
#include "DiffHelper.h"

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
#include <QFileSystemWatcher>
#include <QDirIterator>

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
    QFileSystemWatcher *watcher{nullptr};
    QString reposPath;
    QString name;
    QString passwd;
    HistoryData currHistoryData;
    RevisionFile currRevisonFile;
    bool logginResult = false;
};

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

    d->watcher = new QFileSystemWatcher(this);

    QObject::connect(d->watcher, &QFileSystemWatcher::directoryChanged,
                     [=](const QString &filePath){
        this->reloadRevisionFiles();
    });
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

    QDirIterator dirItera(d->reposPath, QDir::Filter::NoDotAndDotDot|QDir::Dirs|QDir::NoSymLinks);
    while (dirItera.hasNext()) {
        dirItera.next();
        qInfo() << dirItera.filePath();
        d->watcher->addPath(dirItera.filePath());
    }
    d->watcher->addPath(path);
    d->fileSrcView->setRootPath(d->reposPath);
}

void ReposWidget::loadRevisionFiles()
{
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess process;
    process.setProgram(svnProgram());
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"status"});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
    }

    d->amendsWidget->modView()->setUpdatesEnabled(false);
    RevisionFiles files;
    while (process.canReadLine()) {
        QString line = process.readLine();
        QStringList lineList = line.replace("\n", "").split(" ");
        QString filePath = d->reposPath + QDir::separator() + *lineList.rbegin();
        RevisionFile file(*lineList.rbegin(), filePath, lineList.first());
        d->amendsWidget->modView()->addFile(file);
        files << file;
    }
    d->amendsWidget->modView()->setUpdatesEnabled(true);
}

void ReposWidget::reloadRevisionFiles()
{
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess process;
    process.setProgram(svnProgram());
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"status"});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
    }

    d->amendsWidget->modView()->setUpdatesEnabled(false);
    RevisionFiles newFiles;
    while (process.canReadLine()) {
        QString line = process.readLine();
        QStringList lineList = line.replace("\n", "").split(" ");
        QString filePath = d->reposPath + QDir::separator() + *lineList.rbegin();
        RevisionFile file(*lineList.rbegin(), filePath, lineList.first());
        newFiles << file;
    }

    // to changed added file list
    for(int row = 0; row < d->amendsWidget->modView()->rowCount(); row ++) {
        auto rowFile = d->amendsWidget->modView()->file(row); // row revision file
        if (!newFiles.contains(rowFile)) {
            if (d->amendsWidget->modView()->removeFile(rowFile)) //从界面删除节点
                row --;
        }
        for (auto newFile : newFiles) {
            if (rowFile == newFile) { // 包含则删除 相同数据
                newFiles.removeOne(newFile); // 新的文件中删除当前数据
                break;
            } else if (rowFile.isSameFilePath(newFile) // 路径一致,类型不一致
                       && !rowFile.isSameReviType(newFile)) {
                if (d->amendsWidget->modView()->removeFile(rowFile)) //从界面删除节点
                    row --;
            }
        }
    }
    d->amendsWidget->modView()->addFiles(newFiles);
    d->amendsWidget->modView()->setUpdatesEnabled(true);
}

void ReposWidget::loadHistory()
{
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess process;
    process.setProgram(svnProgram());
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"log", "-v"});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return;
    }

    d->historyWidget->logWidget()->historyView()->setUpdatesEnabled(false);
    HistoryDatas datas;
    QString line = process.readLine();
    while (process.canReadLine()) {
        if (svnLogSplitStr() == line) {
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
                 line != "\n" && line != svnLogSplitStr();
                 line = process.readLine()) {
                descStr += line;
            }
            data.description = descStr;
            descStr.clear();
            datas << data;
            for (;line != svnLogSplitStr(); line = process.readLine()) {

            }
        }
    }
    d->historyWidget->logWidget()->historyView()->setDatas(datas);
    d->historyWidget->logWidget()->historyView()->setUpdatesEnabled(true);
}

void ReposWidget::reloadHistory()
{
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess process;
    process.setProgram(svnProgram());
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"log", "-v"});
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return;
    }

    d->historyWidget->logWidget()->historyView()->setUpdatesEnabled(false);
    HistoryDatas datas;
    QString line = process.readLine();
    while (process.canReadLine()) {
        if (svnLogSplitStr() == line) {
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
                 line != "\n" && line != svnLogSplitStr();
                 line = process.readLine()) {
                descStr += line;
            }
            data.description = descStr;
            descStr.clear();
            if (data == d->historyWidget->logWidget()->historyView()->data(0)) {
                break;
            }
            datas << data;
            for (;line != svnLogSplitStr(); line = process.readLine()) {

            }
        }
    }
    d->historyWidget->logWidget()->historyView()->insertTopDatas(datas);
    d->historyWidget->logWidget()->historyView()->setUpdatesEnabled(true);
}

void ReposWidget::modFileMenu(const RevisionFile &file, const QPoint &pos)
{
    QMenu menu;
    if (file.revisionType == AmendsState_Col1::get()->ADD) {
        QAction *action = menu.addAction("revert");
        QObject::connect(action, &QAction::triggered, [=](){
            this->revert(file.displayName);
            this->reloadRevisionFiles();
        });
    }
    if (file.revisionType == AmendsState_Col1::get()->SRC) {
        QAction *action = menu.addAction("add");
        QObject::connect(action, &QAction::triggered, [=](){
            this->add(file.displayName);
            this->reloadRevisionFiles();
        });
    }
    menu.exec(pos);
}

void ReposWidget::historyDataClicked(const QModelIndex &index)
{
    auto hisView = d->historyWidget->logWidget()->historyView();
    d->currHistoryData = hisView->data(index.row());
}

void ReposWidget::historyFileClicked(const QModelIndex &index)
{
    auto chaView = d->historyWidget->logWidget()->fileChangedView();
    d->currRevisonFile = chaView->file(index.row());
    if (d->currHistoryData.isValid() && d->currRevisonFile.isValid()) {
        doDiffFileAtRevision();
    }
}

void ReposWidget::setSrcViewReviFiles(const QString &path)
{
    qInfo() << path;
    reloadRevisionFiles();
    d->amendsWidget->modView()->files();
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

    QObject::connect(d->amendsWidget->modView(),
                     &FileModifyView::menuRequest,
                     this, &ReposWidget::modFileMenu);

    // 历史记录操作
    auto hisView = d->historyWidget->logWidget()->historyView();
    QObject::connect(hisView, &HistoryView::clicked,
                     this, &ReposWidget::historyDataClicked);

    auto chaView = d->historyWidget->logWidget()->fileChangedView();
    QObject::connect(chaView, &FileModifyView::clicked,
                     this, &ReposWidget::historyFileClicked);

    loadRevisionFiles(); // 创建修订文件
    loadHistory(); // 添加提交历史

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
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess processUpdate;
    processUpdate.setWorkingDirectory(d->reposPath);
    processUpdate.setProgram(svnProgram());
    processUpdate.setArguments({"update"});
    processUpdate.start();
    processUpdate.waitForFinished();
    if (processUpdate.exitCode() != 0 || processUpdate.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(processUpdate.readAllStandardError());
    }
    doRefresh();
}

void ReposWidget::doRefresh()
{
    reloadRevisionFiles();
    reloadHistory();
}

void ReposWidget::doAmendsCommit()
{
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess processCommit;
    processCommit.setWorkingDirectory(d->reposPath);
    processCommit.setProgram(svnProgram());
    QString commitDesc = d->amendsWidget->description();
    processCommit.setArguments({"commit", "-m", commitDesc, "--username", d->name, "--password", d->passwd});
    processCommit.start();
    processCommit.waitForFinished();
    if (processCommit.exitCode() != 0 || processCommit.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(processCommit.readAllStandardError());
    }
    reloadRevisionFiles();
}

void ReposWidget::doAmendsRevertAll()
{
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess process;
    process.setProgram(svnProgram());
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"revert", d->reposPath, "--depth", "infinity"});
    process.start();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return;
    }
    qInfo() << process.errorString() << QString::fromUtf8(process.readAll());
    reloadRevisionFiles();
}

void ReposWidget::doDiffFileAtRevision()
{
    if (svnProgram().isEmpty()) {
        return;
    }
    QProcess process;
    process.setProgram(svnProgram());
    process.setWorkingDirectory(d->reposPath);
    process.setArguments({"diff", "--git", d->currRevisonFile.displayName, "-r", d->currHistoryData.revision});
    process.start();
    process.waitForFinished();
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::ExitStatus::NormalExit) {
        ContextDialog::ok(process.readAllStandardError());
        return;
    }

    QString text = process.readAll();
    QPair<QStringList, QVector<ChunkDiffInfo::ChunkInfo>> newFileData;
    QPair<QStringList, QVector<ChunkDiffInfo::ChunkInfo>> oldFileData;
    auto info = DiffHelper::processDiff(text, newFileData, oldFileData);

    d->historyWidget->diffWidget()->getOldView()->getDiffView()->loadDiff(oldFileData.first.join('\n'), oldFileData.second);
    d->historyWidget->diffWidget()->getNewView()->getDiffView()->loadDiff(newFileData.first.join('\n'), newFileData.second);
    qInfo() << "jump";
}

bool ReposWidget::testUserLoggin(const QString &reposPath, const QString &name, const QString &passwd)
{
    if (svnProgram().isEmpty()) {
        return false;
    }
    QProcess process;
    process.setProgram(svnProgram());
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
    QObject::connect(d->updateButton, &QToolButton::clicked, this, &ReposWidget::doUpdateRepos);
    d->controlBar->addWidget(d->updateButton);

    d->refreshButton = new QToolButton();
    d->refreshButton->setFixedSize(buttonWidth, buttonHeight);
    d->refreshButton->setIcon(QIcon(":/icons/refresh"));
    d->refreshButton->setToolTip(QToolButton::tr("refresh current local to display"));
    QObject::connect(d->refreshButton, &QToolButton::clicked, this, &ReposWidget::doRefresh);
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

    d->optionButton->setChecked(true);

    return d->controlBar;
}

bool ReposWidget::add(const QString &display)
{
    if (svnProgram().isEmpty()) {
        return false;
    }
    QProcess process;
    process.setProgram(svnProgram());
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
    if (svnProgram().isEmpty()) {
        return false;
    }
    QProcess process;
    process.setProgram(svnProgram());
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
    d->logginDialog->setName(value);
}

QString ReposWidget::getPasswd() const
{
    return d->passwd;
}

void ReposWidget::setPasswd(const QString &value)
{
    d->passwd = value;
    d->logginDialog->setPasswd(value);
}

void ReposWidget::setLogginDisplay(const QString &name, const QString &passwd)
{
    d->logginDialog->setName(name);
    d->logginDialog->setPasswd(passwd);
}
