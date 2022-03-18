#include "recentdisplay.h"

#include "displayrecentview.h"
#include "transceiver/sendevents.h"

#include <QList>
#include <QDir>
#include <QStandardItemModel>
#include <QListView>
#include <QHBoxLayout>

static RecentDisplay *ins{nullptr};

class DisplayDirView : public DisplayRecentView
{
public:
    explicit DisplayDirView(QWidget *parent = nullptr)
        : DisplayRecentView(parent)
    {
        load();
    }

    virtual QString title() override
    {
        return "Folders";
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
        return "Documents";
    }
};

class RecentDisplayPrivate
{
    friend class RecentDisplay;
    QHBoxLayout *hLayout{nullptr};
    QVBoxLayout *vLayoutDoc{nullptr};
    QVBoxLayout *vLayoutDir{nullptr};
    DisplayDirView *dirView{nullptr};
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
    d->docLabel = new QLabel(d->docView->title());
    d->vLayoutDoc = new QVBoxLayout();
    d->vLayoutDoc->setMargin(40);
    d->vLayoutDoc->addWidget(d->docLabel);
    d->vLayoutDoc->setSpacing(20);
    d->vLayoutDoc->addWidget(d->docView);
    d->vLayoutDoc->setAlignment(d->dirView, Qt::AlignRight);

    d->dirView = new DisplayDirView();
    d->dirView->setMinimumWidth(400);
    d->dirLabel = new QLabel(d->dirView->title());
    d->vLayoutDir = new QVBoxLayout();
    d->vLayoutDir->setMargin(40);
    d->vLayoutDir->addWidget(d->dirLabel);
    d->vLayoutDir->setSpacing(20);
    d->vLayoutDir->addWidget(d->dirView);
    d->vLayoutDir->setAlignment(d->docView, Qt::AlignLeft);

    d->hLayout = new QHBoxLayout();
    d->hLayout->addStretch();
    d->hLayout->addLayout(d->vLayoutDir);
    d->hLayout->addLayout(d->vLayoutDoc);
    d->hLayout->addStretch();
    setLayout(d->hLayout);

    QObject::connect(d->dirView, &QListView::doubleClicked, this, [=](const QModelIndex &index){
        QString filePath = index.data(Qt::DisplayRole).toString();
        emit RecentDisplay::doubleClickedFolder(filePath);
        SendEvents::recentOpenDirectory(filePath);
    });

    QObject::connect(d->docView, &QListView::doubleClicked, this, [=](const QModelIndex &index){
        QString filePath = index.data(Qt::DisplayRole).toString();
        emit RecentDisplay::doubleClickedDocument(filePath);
        SendEvents::recentOpenFile(filePath);
    });
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

void RecentDisplay::addFolder(const QString &filePath)
{
    d->dirView->add(filePath);
}
