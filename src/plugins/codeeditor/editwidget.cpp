#include "editwidget.h"
#include "editfiletabwidget.h"
#include "editfilestatusbar.h"
#include "edittextwidget.h"
#include "codeeditorreceiver.h"

#include <QGridLayout>
#include <QFileInfo>

class EditWidgetPrivate
{
    friend class EditWidget;
    EditFileTabWidget *tab = nullptr;
    QGridLayout *gridLayout = nullptr;
    QHash<QString, EditTextWidget*> textEdits;
    QHash<QString, EditFileStatusBar*> statusBars;
    EditTextWidget defaultEdit;
};

EditWidget::EditWidget(QWidget *parent)
    : QWidget(parent)
    , d(new EditWidgetPrivate)
{
    d->gridLayout = new QGridLayout(this);
    d->gridLayout->setSpacing(0);
    d->gridLayout->setMargin(0);

    if (!d->tab) {
        d->tab = new EditFileTabWidget(this);
    }

    d->gridLayout->addWidget(d->tab);
    d->gridLayout->addWidget(&d->defaultEdit);
    this->setLayout(d->gridLayout);

    setDefaultFileEdit();

    QObject::connect(d->tab, &EditFileTabWidget::currentChanged,
                     this, &EditWidget::showFileEdit);

    QObject::connect(&DpfEventMiddleware::instance(), &DpfEventMiddleware::toOpenFile,
                     this, &EditWidget::openFile);

    QObject::connect(d->tab, &EditFileTabWidget::tabCloseRequested,
                     this, &EditWidget::removeFileEdit, Qt::QueuedConnection);

    QObject::connect(d->tab, &EditFileTabWidget::tabCloseRequested,
                     this, &EditWidget::removeFileTab, Qt::QueuedConnection);
}

EditWidget::~EditWidget()
{
    if (d) {
        if (d->tab) {
            delete d->tab;
            d->tab = nullptr;
        }
        auto itera = d->textEdits.begin();
        while (itera != d->textEdits.end()){
            delete itera.value(); // free instance
            *itera = nullptr; // set instance nullptr
            itera ++;
        }
        d->textEdits.clear(); // clear hash
        delete d; // free private
    }
}

int EditWidget::tabIndex(const QString &tabTooltip)
{
    int tabCount = d->tab->count();
    for (int index = 0; index < tabCount; index++) {
        if(d->tab->tabToolTip(index) == tabTooltip) {
            return index;
        }
    }
    return -1;
}

void EditWidget::openFile(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists() || !d->tab)
        return;

    // can't add widget to much
    if (d->textEdits.keys().contains(info.filePath())) {
        d->tab->setCurrentIndex(tabIndex(info.filePath()));
        return;
    }

    int idx = d->tab->addTab(info.fileName());
    d->tab->setTabToolTip(idx, info.filePath());
    d->tab->setTabsClosable(true);
    d->tab->setCurrentIndex(idx);
    EditTextWidget *edit = new EditTextWidget;
    d->textEdits.insert(info.filePath(), edit);
    edit->setCurrentFile(info.filePath());

    // set display textedit
    d->gridLayout->addWidget(edit);

    if (!d->defaultEdit.isHidden())
        d->defaultEdit.hide();

    showFileEdit(idx);
}

void EditWidget::closeFile(const QString &filePath)
{
    if (!d->tab)
        return;
    int index = tabIndex(filePath);
    if (index >=0 && index < d->tab->count())
        emit d->tab->tabCloseRequested(index);

}

void EditWidget::setDefaultFileEdit()
{
    if (!d || !d->gridLayout)
        return;

    d->gridLayout->addWidget(&d->defaultEdit);
    d->defaultEdit.setEnabled(false);
    d->defaultEdit.show();
}

// Edit多开准备
void EditWidget::hideFileEdit(int tabIndex)
{
    if (!d->gridLayout)
        return;

    QString filePath = d->tab->tabToolTip(tabIndex);
    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    edit->hide();
}

void EditWidget::showFileEdit(int tabIndex)
{
    if (!d->gridLayout)
        return;

    QString filePath = d->tab->tabToolTip(tabIndex);
    auto edit = d->textEdits.value(filePath);

    if (!edit)
        return;

    auto itera = d->textEdits.begin();
    while (itera != d->textEdits.end()){
        if (itera.key() != d->tab->tabToolTip(tabIndex)){
            itera.value()->hide(); // hide other;
        }
        itera ++;
    }

    edit->show();
}

//Edit多开准备
void EditWidget::hideFileStatusBar(int tabIndex)
{

}

void EditWidget::showFileStatusBar(int tabIndex)
{

}

void EditWidget::removeFileEdit(int tabIndex)
{
    QString filePath = d->tab->tabToolTip(tabIndex);
    auto edit = d->textEdits.value(filePath);
    if (!edit)
        return;

    delete edit;
    d->textEdits.remove(filePath);

    if (d->textEdits.size() == 0)
        setDefaultFileEdit();
}

void EditWidget::removeFileTab(int tabIndex)
{
    if (!d->tab)
        return;

    d->tab->removeTab(tabIndex);
}
