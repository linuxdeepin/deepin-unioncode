#include "filelistwidget.h"
#include "GitQlientStyles.h"
#include "FileListDelegate.h"
#include "FileContextMenu.h"

namespace collaborators {

FileListWidget::FileListWidget(QWidget *parent)
    : QListWidget (parent)
    , mFileDelegate(new FileListDelegate(this))
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setItemDelegate(mFileDelegate);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(this, &FileListWidget::customContextMenuRequested, this, &FileListWidget::showContextMenu);
}

void FileListWidget::insertFiles(const std::optional<RevisionFiles> &files)
{
    clear();

    setUpdatesEnabled(false);

    for (auto i = 0; i < files->count(); ++i)
    {
        if (!files->statusCmp(i, RevisionFiles::UNKNOWN)) {
            QColor clr;
            QString fileName;
            if (files->statusCmp(i, RevisionFiles::NEW)) {
                const auto fileRename = files->extendedStatus(i);

                clr = fileRename.isEmpty() ? GitQlientStyles::getGreen() : GitQlientStyles::getBlue();
                fileName = fileRename.isEmpty() ? files->getFile(i) : fileRename;
            } else {
                clr = files->statusCmp(i, RevisionFiles::DELETED) ? GitQlientStyles::getRed()
                                                                  : GitQlientStyles::getTextColor();
                fileName = files->getFile(i);
            }
            addItem(fileName, clr);
        }
    }

    setUpdatesEnabled(true);
}

void FileListWidget::showContextMenu(const QPoint &pos)
{
    const auto item = itemAt(pos);

    if (item)
    {
       const auto fileName = item->data(Qt::DisplayRole).toString();
       const auto menu = new FileContextMenu(fileName, false, this);
       connect(menu, &FileContextMenu::signalShowFileHistory, this,
               [this, fileName]() { emit signalShowFileHistory(fileName); });
       connect(menu, &FileContextMenu::signalOpenFileDiff, this,
               [this, item] { emit QListWidget::itemDoubleClicked(item); });
       menu->exec(viewport()->mapToGlobal(pos));
    }
}

void FileListWidget::addItem(const QString &label, const QColor &clr)
{
    const auto item = new QListWidgetItem(label, this);
    item->setForeground(clr);
    item->setToolTip(label);
}

} // namespace collaborators
