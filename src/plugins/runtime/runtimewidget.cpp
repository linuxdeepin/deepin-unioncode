#include "runtimewidget.h"
#include "configurewidget.h"
#include "environmentwidget.h"
#include "common/widget/collapsewidget.h"

#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStyleFactory>

class RuntimeWidgetPrivate
{
    friend class RuntimeWidget;
    QVBoxLayout *leftLayout = nullptr;
    QVBoxLayout *rightLayout = nullptr;
    QGroupBox *folderGroupBox = nullptr;
    QComboBox *openFolderBox = nullptr;
    QPushButton *openNewFolder = nullptr;
    ConfigureWidget *configWidget = nullptr;
};

RuntimeWidget::RuntimeWidget(QWidget *parent)
    : QSplitter (parent)
    , d(new RuntimeWidgetPrivate())
{
    if (!d->leftLayout)
        d->leftLayout = new QVBoxLayout();

    if (!d->folderGroupBox)
        d->folderGroupBox = new QGroupBox(tr("Open Folder"));
    d->folderGroupBox->setLayout(d->leftLayout);

    if (!d->openFolderBox)
        d->openFolderBox = new QComboBox();
    d->leftLayout->addWidget(d->openFolderBox, 0, Qt::AlignTop);

    if (!d->openNewFolder)
        d->openNewFolder = new QPushButton(tr("Open New Folder"));
    d->leftLayout->addWidget(d->openNewFolder, 0, Qt::AlignTop);
    d->leftLayout->addStretch();

    // test project name
    d->openFolderBox->addItems({"unioncode", "dde-file-manager"});

    if (!d->configWidget)
        d->configWidget = new ConfigureWidget();
    d->configWidget->addCollapseWidget(new CollapseWidget("Runtime Env", new EnvironmentWidget));
    d->configWidget->addCollapseWidget(new CollapseWidget("Debug Env", new EnvironmentWidget));

    addWidget(d->folderGroupBox);
    addWidget(d->configWidget);
    setStretchFactor(1, 3);

    setChildrenCollapsible(false);
}

RuntimeWidget::~RuntimeWidget()
{
    if (d)
        delete d;
}
