#include "basetype.h"

#include "common/common.h"

#include "filemodifywidget.h"
#include "GitQlientStyles.h"
#include "FileListDelegate.h"
#include "FileContextMenu.h"

#include <QPushButton>

FileModifyWidget::FileModifyWidget(QWidget *parent)
    : QListWidget (parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(this, &FileModifyWidget::customContextMenuRequested, this, &FileModifyWidget::showContextMenu);
}

RevisionFile FileModifyWidget::file(int index)
{
    if (index >= 0 && index < count()) {
        auto aItem = item(index);
        return createRevi(aItem);
    }
    return {};
}

void FileModifyWidget::addFile(const RevisionFile &file)
{
    setUpdatesEnabled(false);
    addItem(FileModifyWidget::createItem(file));
    setUpdatesEnabled(true);
}

void FileModifyWidget::addFiles(const RevisionFiles &files)
{
    setUpdatesEnabled(false);

    for (auto file : files) {
        addItem(FileModifyWidget::createItem(file));
    }

    setUpdatesEnabled(false);
}

void FileModifyWidget::setFiles(const RevisionFiles &files)
{
    clear();
    addFiles(files);
}

void FileModifyWidget::removeFile(const RevisionFile &file)
{
    for (int row = 0; row < count(); row++) {
        if (createRevi(item(row)) == file) {
            auto rowItem = takeItem(row);
            if (rowItem)
                delete rowItem;
        }
    }
}

void FileModifyWidget::showContextMenu(const QPoint &pos)
{
    const auto item = itemAt(pos);
    if (item) {
        RevisionFile file(
                    item->data(FileModifyRole::FilePath).toString(),
                    item->data(FileModifyRole::FileIconType).toString(),
                    item->data(FileModifyRole::RevisionType).toString()
                    );
        fileMenuRequest(file, viewport()->mapToGlobal(pos));
    }
}

QListWidgetItem *FileModifyWidget::createItem(const RevisionFile &file)
{
    QFileInfo info(file.filePath);
    auto item = new QListWidgetItem(file.displayName);

    QIcon icon;
    if (info.isDir()) {
        item->setData(FileModifyRole::FileIconType, QFileIconProvider::Folder);
        icon = CustomIcons::icon(QFileIconProvider::Folder);
    } else if (info.isFile()) {
        item->setData(FileModifyRole::FileIconType, QFileIconProvider::File);
        icon = CustomIcons::icon(QFileIconProvider::File);
    }
    item->setData(FileModifyRole::FilePath, file.filePath);
    item->setData(FileModifyRole::RevisionType, file.revisionType);
    item->setIcon(icon);
    item->setToolTip(file.filePath);
    return item;
}

RevisionFile FileModifyWidget::createRevi(const QListWidgetItem *item)
{
    return {    item->data(Qt::DisplayRole).toString(),
                item->data(FileModifyRole::FilePath).toString(),
                item->data(FileModifyRole::RevisionType).toString()   };
}
