// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentdisplay.h"
#include "displayrecentview.h"
#include "services/window/windowservice.h"
#include "plugins/template/wizard/maindialog.h"

#include <DLabel>
#include <DFrame>
#include <DStyle>
#include <DPushButton>
#include <DFileDialog>

#include <QImageReader>
#include <QList>
#include <QDir>
#include <QStandardItemModel>
#include <QListView>
#include <QHBoxLayout>
#include <QJsonDocument>

DWIDGET_USE_NAMESPACE

static RecentDisplay *ins{nullptr};

class DisplayProView : public DisplayRecentView
{
    QJsonArray projects;
public:
    explicit DisplayProView(QWidget *parent = nullptr)
        : DisplayRecentView(parent)
    {
        load();
    }

    virtual QString title() override
    {
        return tr("Projects");
    }

    virtual QList<QStandardItem*> itemsFromFile() override
    {
        QJsonDocument doc = readRecent();
        QJsonObject obj = doc.object();
        QJsonArray array = obj.value(title()).toArray();
        QList<QStandardItem*> result;
        QStringList paths = cachedWorkspaces(projects);
        for (auto val : array) {
            auto elemObj = val.toObject();
            QString language, workspace, kitName;
            QString file = projectFile(elemObj, &kitName, &language, &workspace);
            if (file.isEmpty() || !QFileInfo(file).exists())
                continue;
            auto rowItem = new QStandardItem (icon(file), file);
            rowItem->setData(kitName, RecentDisplay::ProjectKitName);
            rowItem->setData(language, RecentDisplay::ProjectLanguage);
            rowItem->setData(workspace, RecentDisplay::ProjectWorkspace);
            rowItem->setToolTip( "KitName: " + kitName + "\n" +
                                 "Language: " + language + "\n" +
                                 "Workspace: " + workspace );
            if (!paths.contains(file))
                projects << val;
            result << rowItem;
        }
        return result;
    }

    virtual void load() override
    {
        model->appendColumn(itemsFromFile());
    }

    virtual void add(const QString &kitName,
                     const QString &language,
                     const QString &workspace)
    {
        model->clear(); //删除数据
        auto paths = cachedWorkspaces(projects);
        for (auto path : paths) {
            if (!QFileInfo(path).exists()) {
                removeProjectElem(projects, path);
            }
        }
        if (paths.contains(workspace)) {
            removeProjectElem(projects, workspace);
        }
        projects.insert(0, projectElem(kitName, language, workspace)); //置顶
        saveToFile(projects); //保存序列
        load(); //重新加载文件
    }

    virtual QString projectFile(const QJsonObject &elem,
                                QString *kitName = nullptr,
                                QString *language = nullptr,
                                QString *workspace = nullptr)
    {
        if (elem.keys().size() != 1)
            return {};

        QString file = elem.keys()[0];

        if (file.isEmpty())
            return {};

        auto propertyObj = elem.value(file).toObject();
        if (language)
            *kitName = propertyObj["KitName"].toString();

        if (language)
            *language = propertyObj["Language"].toString();

        if (workspace)
            *workspace = propertyObj["Workspace"].toString();

        return file;
    }

    virtual QJsonObject projectElem(const QString &kitName,
                                    const QString &language,
                                    const QString &workspace)
    {
        QJsonObject elem;
        QJsonObject propertyVal;
        propertyVal.insert("KitName", kitName);
        propertyVal.insert("Language", language);
        propertyVal.insert("Workspace", workspace);
        elem.insert(workspace, QJsonValue{propertyVal});
        return elem;
    }

    virtual QStringList cachedWorkspaces(const QJsonArray &array)
    {
        QStringList list;
        for (auto val : array){
            list += val.toObject().keys();
        }
        return list;
    }

    virtual void removeProjectElem(QJsonArray &array, const QString &path)
    {
        if (path.isEmpty())
            return;

        for (int i = 0; i < array.size(); i++) {
            auto keys = array[i].toObject().keys();
            if (keys.size() != 1)
                continue;

            QString currPath = keys[0];
            if (currPath.isEmpty()) {
                return;
            }
            if (currPath == path) {
                array.removeAt(i);
            }
        }
    }

    virtual void saveToFile(const QJsonArray &array)
    {
        QJsonDocument doc = readRecent();
        QJsonObject docObj = doc.object();
        docObj[title()] = array;
        doc.setObject(docObj);

        QFile file(cachePath());
        if (file.open(QFile::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }
};

class DisplayDocView : public DisplayRecentView
{
public:
    explicit DisplayDocView(QWidget *parent = nullptr)
        : DisplayRecentView(parent){
        load();
    }

    virtual QString title() override
    {
        return tr("Documents");
    }
};

class RecentDisplayPrivate
{
    friend class RecentDisplay;
    QHBoxLayout *hLayout{nullptr};
    QVBoxLayout *vLayoutDoc{nullptr};
    QVBoxLayout *vLayoutDir{nullptr};
    DisplayProView *proView{nullptr};
    DisplayDocView *docView{nullptr};
    DLabel *dirLabel{nullptr};
    DLabel *docLabel{nullptr};

    DPushButton *btnOpenFile{nullptr};
    DPushButton *btnOpenProject{nullptr};
    DPushButton *btnNewFileOrPro{nullptr};
};

RecentDisplay::RecentDisplay(DWidget *parent)
    : DWidget (parent)
    , d(new RecentDisplayPrivate())
{
    DFrame *navFrame = new DFrame();
    DFrame *docFrame = new DFrame();
    DFrame *proFrame = new DFrame();

    navFrame = new DFrame();
    navFrame->setLineWidth(0);
    navFrame->setContentsMargins(0, 0, 0, 0);
    DStyle::setFrameRadius(navFrame, 0);
    QVBoxLayout *vLayoutNav = new QVBoxLayout();
    QLabel *recentLogo = new QLabel();
    QImageReader maskIimageReader(":/recent/images/recentLogo1.png");
    maskIimageReader.setScaledSize(QSize(200, 143));
    QPixmap logo = QPixmap::fromImage(maskIimageReader.read());
    logo.setDevicePixelRatio(recentLogo->devicePixelRatioF());
    recentLogo->setPixmap(logo);
    d->btnOpenFile = new DPushButton(tr("Open File"));
    d->btnOpenProject = new DPushButton(tr("Open Project"));
    d->btnNewFileOrPro = new DPushButton(tr("New File or Project"));
    vLayoutNav->setContentsMargins(50, 200, 50, 200);
    vLayoutNav->setSpacing(20);
    vLayoutNav->setAlignment(Qt::AlignCenter);
    vLayoutNav->addWidget(recentLogo);
    vLayoutNav->addWidget(d->btnOpenFile);
    vLayoutNav->addWidget(d->btnOpenProject);
    vLayoutNav->addWidget(d->btnNewFileOrPro);
    navFrame->setLayout(vLayoutNav);

    DLabel *recentTitle = new DLabel(tr("Recent Open"));
    recentTitle->setForegroundRole(QPalette::BrightText);
    recentTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(recentTitle, DFontSizeManager::T4, QFont::Medium);

    docFrame->setLineWidth(0);
    d->docView = new DisplayDocView();
    d->docLabel = new DLabel(tr("Documents"));
    d->docLabel->setForegroundRole(QPalette::BrightText);
    d->docLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(d->docLabel, DFontSizeManager::T4, QFont::Medium);
    d->vLayoutDoc = new QVBoxLayout();
    d->vLayoutDoc->setContentsMargins(10, 10, 10, 10);
    d->vLayoutDoc->addWidget(d->docLabel);
    d->vLayoutDoc->setSpacing(20);
    d->vLayoutDoc->addWidget(d->docView);
    docFrame->setLayout(d->vLayoutDoc);

    proFrame->setLineWidth(0);
    d->proView = new DisplayProView();
    d->dirLabel = new DLabel(tr("Projects"));
    d->dirLabel->setForegroundRole(QPalette::BrightText);
    d->dirLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(d->dirLabel, DFontSizeManager::T4, QFont::Medium);
    d->vLayoutDir = new QVBoxLayout();
    d->vLayoutDir->setContentsMargins(10, 10, 10, 10);
    d->vLayoutDir->addWidget(d->dirLabel);
    d->vLayoutDir->setSpacing(20);
    d->vLayoutDir->addWidget(d->proView);
    proFrame->setLayout(d->vLayoutDir);

    QHBoxLayout *proAndDocLayout = new QHBoxLayout();
    proAndDocLayout->addWidget(proFrame);
    proAndDocLayout->addSpacing(0);
    proAndDocLayout->addWidget(docFrame);

    QVBoxLayout *recentNavLayout = new QVBoxLayout();
    recentNavLayout->setContentsMargins(20, 0, 25, 20);
    recentNavLayout->addSpacing(15);
    recentNavLayout->setAlignment(Qt::AlignTop);
    recentNavLayout->addWidget(recentTitle);
    recentNavLayout->addSpacing(15);
    recentNavLayout->addLayout(proAndDocLayout);

    d->hLayout = new QHBoxLayout();
    d->hLayout->setContentsMargins(0, 0, 0, 0);
    d->hLayout->addWidget(navFrame);
    d->hLayout->addLayout(recentNavLayout);
    setLayout(d->hLayout);

    QObject::connect(d->proView, &QListView::doubleClicked,
                     this, &RecentDisplay::doDoubleClickedProject,
                     Qt::UniqueConnection);

    QObject::connect(d->docView, &QListView::doubleClicked,
                     this, &RecentDisplay::doDoubleCliekedDocument,
                     Qt::UniqueConnection);

    QObject::connect(d->btnOpenFile, &DPushButton::clicked,
                     this, &RecentDisplay::btnOpenFileClicked,
                     Qt::UniqueConnection);

    QObject::connect(d->btnOpenProject, &DPushButton::clicked,
                     this, &RecentDisplay::btnOpenProjectClicked,
                     Qt::UniqueConnection);

    QObject::connect(d->btnNewFileOrPro, &DPushButton::clicked,
                     this, &RecentDisplay::btnNewFileOrProClicked,
                     Qt::UniqueConnection);
}

RecentDisplay::~RecentDisplay()
{
    if (d) {
        delete d;
    }
}

RecentDisplay *RecentDisplay::instance()
{
    if (!ins)
        ins = new RecentDisplay;
    return ins;
}

void RecentDisplay::addDocument(const QString &filePath)
{
    d->docView->add(filePath);
}

void RecentDisplay::addProject(const QString &kitName,
                               const QString &language,
                               const QString &workspace)
{
    d->proView->add(kitName, language, workspace);
}

void RecentDisplay::doDoubleClickedProject(const QModelIndex &index)
{
    QString filePath = index.data(Qt::DisplayRole).toString();
    QString kitName = index.data(ProjectKitName).toString();
    QString language = index.data(ProjectLanguage).toString();
    QString workspace = index.data(ProjectWorkspace).toString();
    // "kitName", "language", "workspace"
    project.openProject(kitName, language, workspace);
    RecentDisplay::addProject(kitName, language, workspace);
}

void RecentDisplay::doDoubleCliekedDocument(const QModelIndex &index)
{
    QString filePath = index.data(Qt::DisplayRole).toString();
    RecentDisplay::addDocument(filePath);
    editor.openFile(filePath);
}

void RecentDisplay::btnOpenFileClicked()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getOpenFileName(nullptr, dpfservice::DIALOG_OPEN_DOCUMENT_TITLE, dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;
    recent.saveOpenedFile(filePath);
    editor.openFile(filePath);
}

void RecentDisplay::btnOpenProjectClicked()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getOpenFileName(nullptr, dpfservice::DIALOG_OPEN_DOCUMENT_TITLE, dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;
    recent.saveOpenedFile(filePath);
    editor.openFile(filePath);
}

void RecentDisplay::btnNewFileOrProClicked()
{
    projectTemplate.newWizard();
}

