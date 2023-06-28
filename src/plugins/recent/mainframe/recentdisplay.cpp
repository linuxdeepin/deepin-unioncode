// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentdisplay.h"

#include "displayrecentview.h"

#include <QList>
#include <QDir>
#include <QStandardItemModel>
#include <QListView>
#include <QHBoxLayout>
#include <QJsonDocument>

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
    QLabel *dirLabel{nullptr};
    QLabel *docLabel{nullptr};
};

RecentDisplay::RecentDisplay(QWidget *parent)
    : QWidget (parent)
    , d(new RecentDisplayPrivate())
{
    d->docView = new DisplayDocView();
    d->docView->setMinimumWidth(400);
    d->docLabel = new QLabel(tr("Documents"));
    d->vLayoutDoc = new QVBoxLayout();
    d->vLayoutDoc->setMargin(40);
    d->vLayoutDoc->addWidget(d->docLabel);
    d->vLayoutDoc->setSpacing(20);
    d->vLayoutDoc->addWidget(d->docView);
    d->vLayoutDoc->setAlignment(d->proView, Qt::AlignRight);

    d->proView = new DisplayProView();
    d->proView->setMinimumWidth(400);
    d->dirLabel = new QLabel(tr("Projects"));
    d->vLayoutDir = new QVBoxLayout();
    d->vLayoutDir->setMargin(40);
    d->vLayoutDir->addWidget(d->dirLabel);
    d->vLayoutDir->setSpacing(20);
    d->vLayoutDir->addWidget(d->proView);
    d->vLayoutDir->setAlignment(d->docView, Qt::AlignLeft);

    d->hLayout = new QHBoxLayout();
    d->hLayout->addStretch();
    d->hLayout->addLayout(d->vLayoutDir);
    d->hLayout->addLayout(d->vLayoutDoc);
    d->hLayout->addStretch();
    setLayout(d->hLayout);

    QObject::connect(d->proView, &QListView::doubleClicked,
                     this, &RecentDisplay::doDoubleClickedProject,
                     Qt::UniqueConnection);

    QObject::connect(d->docView, &QListView::doubleClicked,
                     this, &RecentDisplay::doDoubleCliekedDocument,
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

