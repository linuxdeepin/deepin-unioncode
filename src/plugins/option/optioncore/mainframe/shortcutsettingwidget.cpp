// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutsettingwidget.h"
#include "shortcutitem.h"
#include "shortcutdialog.h"

#include "common/actionmanager/actionmanager.h"
#include "base/baseitemdelegate.h"

#include <DSearchEdit>
#include <DToolButton>
#include <DIconButton>

#include <QAction>
#include <QMenu>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSettings>
#include <QFileDialog>

constexpr char kKeyboardShortcuts[] = "KeyboardShortcuts";

DWIDGET_USE_NAMESPACE

QWidget *OptionShortcutsettingGenerator::optionWidget()
{
    return new ShortcutSettingWidget;
}

class ShortcutSettingWidgetPrivate : public QObject
{
public:
    struct Tr
    {
        Q_DECLARE_TR_FUNCTIONS(ShortcutSettingWidget);
    };

    explicit ShortcutSettingWidgetPrivate(ShortcutSettingWidget *qq);

    void initUI();
    void inintConnection();

    void apply();
    void clear();
    void updateCommands();
    void updateShortcut(QTreeWidgetItem *item, const QList<QKeySequence> &keyList);
    void showMenu(const QPoint &pos);

    void addShortcut(QTreeWidgetItem *item);
    void changeShortcut(QTreeWidgetItem *item, int index);
    void removeShortcut(QTreeWidgetItem *item, int index);
    void removeAllShortcut(QTreeWidgetItem *item);
    void resetShortcut(QTreeWidgetItem *item);
    void exportAction();
    void importAction();
    void handleKeyRecord();
    void handleFilterChanged(const QString &f);
    bool filter(const QString &filterString, QTreeWidgetItem *item);
    bool filterColumn(const QString &filterString, QTreeWidgetItem *item, int column);

    ShortcutItem *shortcutItem(QTreeWidgetItem *item);
    int checkConflict(const QKeySequence &key, int index);
    void setModified(QTreeWidgetItem *item, bool modified);
    void resetRecordState();
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    QString createPlaceholderString(const QStringList &list);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

public:
    ShortcutSettingWidget *q;

    DSearchEdit *searchEdit { nullptr };
    DToolButton *recordKeyBtn { nullptr };
    DIconButton *exportBtn { nullptr };
    DIconButton *importBtn { nullptr };
    QTreeWidget *commandWidget { nullptr };
    QList<ShortcutItem *> shortcutItems;

    // for key record
    std::array<int, 4> keyArray { 0 };
    int keyNum { 0 };
};

ShortcutSettingWidgetPrivate::ShortcutSettingWidgetPrivate(ShortcutSettingWidget *qq)
    : q(qq)
{
}

void ShortcutSettingWidgetPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout *hSearchLayout = new QHBoxLayout;
    searchEdit = new DSearchEdit(q);
    searchEdit->setPlaceholderText(Tr::tr("Type to search in keybindings"));
    searchEdit->installEventFilter(this);
    searchEdit->lineEdit()->installEventFilter(this);

    recordKeyBtn = new DToolButton(q);
    recordKeyBtn->setCheckable(true);
    recordKeyBtn->setIcon(QIcon::fromTheme("shortcut_search"));
    recordKeyBtn->setToolTip(Tr::tr("Record Keys"));
    hSearchLayout->addWidget(searchEdit);
    hSearchLayout->addWidget(recordKeyBtn);

    commandWidget = new QTreeWidget(q);
    commandWidget->setHeaderLabels({ Tr::tr("Command"), Tr::tr("Label"), Tr::tr("Shortcut") });
    commandWidget->setColumnWidth(0, 290);
    commandWidget->setColumnWidth(1, 250);
    commandWidget->setAlternatingRowColors(true);
    commandWidget->setIconSize({ 16, 16 });
    commandWidget->setFrameShape(QFrame::NoFrame);
    commandWidget->header()->setStretchLastSection(false);
    commandWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    commandWidget->setMinimumHeight(360);
    commandWidget->setItemDelegate(new BaseItemDelegate(commandWidget));
    commandWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    commandWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    exportBtn = new DIconButton(q);
    exportBtn->setIconSize({ 16, 16 });
    exportBtn->setFlat(true);
    exportBtn->setToolTip(Tr::tr("Export"));
    exportBtn->setIcon(QIcon::fromTheme("export"));

    importBtn = new DIconButton(q);
    importBtn->setIconSize({ 16, 16 });
    importBtn->setFlat(true);
    importBtn->setToolTip(Tr::tr("Import"));
    importBtn->setIcon(QIcon::fromTheme("import"));

    QHBoxLayout *hBtnLayout = new QHBoxLayout;
    hBtnLayout->setAlignment(Qt::AlignRight);
    hBtnLayout->setContentsMargins(0, 6, 20, 6);
    hBtnLayout->setSpacing(10);
    hBtnLayout->addWidget(importBtn);
    hBtnLayout->addWidget(exportBtn);

    DFrame *frame = new DFrame(q);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setSpacing(0);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(commandWidget);
    frameLayout->addWidget(new DHorizontalLine(q));
    frameLayout->addLayout(hBtnLayout);

    mainLayout->addLayout(hSearchLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(frame);
}

void ShortcutSettingWidgetPrivate::inintConnection()
{
    connect(ActionManager::instance(), &ActionManager::commandListChanged,
            this, &ShortcutSettingWidgetPrivate::updateCommands);
    connect(commandWidget, &QTreeWidget::customContextMenuRequested, this, &ShortcutSettingWidgetPrivate::showMenu);
    connect(importBtn, &DIconButton::clicked, this, &ShortcutSettingWidgetPrivate::importAction);
    connect(exportBtn, &DIconButton::clicked, this, &ShortcutSettingWidgetPrivate::exportAction);
    connect(recordKeyBtn, &DToolButton::clicked, this, &ShortcutSettingWidgetPrivate::handleKeyRecord);
    connect(searchEdit, &DSearchEdit::textChanged, this, &ShortcutSettingWidgetPrivate::handleFilterChanged);
}

void ShortcutSettingWidgetPrivate::apply()
{
    for (const auto item : std::as_const(shortcutItems))
        item->cmd->setKeySequences(item->shortcutKeys);
}

void ShortcutSettingWidgetPrivate::clear()
{
    for (int i = commandWidget->topLevelItemCount() - 1; i >= 0; --i) {
        delete commandWidget->takeTopLevelItem(i);
    }

    qDeleteAll(shortcutItems);
    shortcutItems.clear();
}

void ShortcutSettingWidgetPrivate::updateCommands()
{
    clear();
    QMap<QString, QTreeWidgetItem *> sectionMap;

    auto commands = ActionManager::instance()->commandList();
    for (auto cmd : commands) {
        if (cmd->hasAttribute(Command::CA_NonConfigurable))
            continue;
        if (cmd->action() && cmd->action()->isSeparator())
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setSizeHint(0, { 0, 24 });
        ShortcutItem *shortcutItem = new ShortcutItem;
        shortcutItem->cmd = cmd;
        shortcutItem->item = item;
        shortcutItem->shortcutKeys = cmd->keySequences();

        const QString identifier = cmd->id();
        int pos = identifier.indexOf(QLatin1Char('.'));
        const QString section = identifier.left(pos);
        const QString subId = identifier.mid(pos + 1);
        if (!sectionMap.contains(section)) {
            auto groupItem = new QTreeWidgetItem(commandWidget, QStringList(section));
            sectionMap.insert(section, groupItem);
            groupItem->setIcon(0, QIcon::fromTheme("shortcut_group"));
            commandWidget->expandItem(groupItem);
        }
        sectionMap[section]->addChild(item);

        item->setData(0, Qt::UserRole, QVariant::fromValue(shortcutItem));
        item->setText(0, subId);
        item->setText(1, cmd->description());
        updateShortcut(item, shortcutItem->shortcutKeys);
        shortcutItems << shortcutItem;
    }

    commandWidget->sortByColumn(0, Qt::AscendingOrder);
}

void ShortcutSettingWidgetPrivate::updateShortcut(QTreeWidgetItem *item, const QList<QKeySequence> &keyList)
{
    auto scItem = shortcutItem(item);
    if (!scItem)
        return;

    setModified(item, scItem->cmd->defaultKeySequences() != keyList);
    scItem->shortcutKeys = keyList;
    commandWidget->removeItemWidget(item, 2);
    if (keyList.isEmpty()) {
        item->setText(2, "");
        return;
    }

    const auto &keyStrList = Command::keySequencesToNativeString(scItem->shortcutKeys);
    ShortcutLabel *keyLabel = new ShortcutLabel(q);
    keyLabel->setKeySqueces(keyStrList);
    commandWidget->setItemWidget(item, 2, keyLabel);

    // for auto-adjusting the column width
    item->setText(2, createPlaceholderString(keyStrList));
}

void ShortcutSettingWidgetPrivate::showMenu(const QPoint &pos)
{
    auto item = commandWidget->itemAt(pos);
    auto scItem = shortcutItem(item);
    if (!scItem)
        return;

    QMenu menu;
    menu.addAction(Tr::tr("Add Shortcut"), this, std::bind(&ShortcutSettingWidgetPrivate::addShortcut, this, item));

    // change action
    for (int i = 0; i < scItem->shortcutKeys.size(); ++i) {
        QString keyText = Tr::tr("Change %1");
        menu.addAction(keyText.arg(scItem->shortcutKeys[i].toString(QKeySequence::NativeText)),
                       this, std::bind(&ShortcutSettingWidgetPrivate::changeShortcut, this, item, i));
    }
    menu.addSeparator();
    // remove action
    for (int i = 0; i < scItem->shortcutKeys.size(); ++i) {
        QString keyText = Tr::tr("Remove %1");
        menu.addAction(keyText.arg(scItem->shortcutKeys[i].toString(QKeySequence::NativeText)),
                       this, std::bind(&ShortcutSettingWidgetPrivate::removeShortcut, this, item, i));
    }
    menu.addSeparator();
    if (scItem->shortcutKeys.size() > 1)
        menu.addAction(Tr::tr("Remove All Shortcut"), this, std::bind(&ShortcutSettingWidgetPrivate::removeAllShortcut, this, item));

    if (scItem->shortcutKeys != scItem->cmd->defaultKeySequences())
        menu.addAction(Tr::tr("Reset Shortcut"), this, std::bind(&ShortcutSettingWidgetPrivate::resetShortcut, this, item));

    menu.exec(QCursor::pos());
}

void ShortcutSettingWidgetPrivate::addShortcut(QTreeWidgetItem *item)
{
    auto scItem = shortcutItem(item);
    if (!scItem)
        return;

    ShortcutDialog dlg;
    dlg.setConflictCheckHandler(std::bind(&ShortcutSettingWidgetPrivate::checkConflict,
                                          this, std::placeholders::_1, scItem->shortcutKeys.size()));
    if (dlg.exec() != 1)
        return;

    const auto &key = dlg.keySequece();
    scItem->shortcutKeys << key;
    updateShortcut(item, scItem->shortcutKeys);
}

void ShortcutSettingWidgetPrivate::changeShortcut(QTreeWidgetItem *item, int index)
{
    auto scItem = shortcutItem(item);
    if (!scItem)
        return;

    ShortcutDialog dlg;
    dlg.setConflictCheckHandler(std::bind(&ShortcutSettingWidgetPrivate::checkConflict,
                                          this, std::placeholders::_1, index));
    if (dlg.exec() != 1)
        return;

    const auto &key = dlg.keySequece();
    scItem->shortcutKeys.replace(index, key);
    updateShortcut(item, scItem->shortcutKeys);
}

void ShortcutSettingWidgetPrivate::removeShortcut(QTreeWidgetItem *item, int index)
{
    auto scItem = shortcutItem(item);
    if (!scItem || index >= scItem->shortcutKeys.size())
        return;

    scItem->shortcutKeys.removeAt(index);
    updateShortcut(item, scItem->shortcutKeys);
}

void ShortcutSettingWidgetPrivate::removeAllShortcut(QTreeWidgetItem *item)
{
    updateShortcut(item, {});
}

void ShortcutSettingWidgetPrivate::resetShortcut(QTreeWidgetItem *item)
{
    auto scItem = shortcutItem(item);
    if (!scItem)
        return;

    updateShortcut(item, scItem->cmd->defaultKeySequences());
}

void ShortcutSettingWidgetPrivate::exportAction()
{
    const auto &filePath = QFileDialog::getSaveFileName(q, Tr::tr("Export Keyboard Mapping Scheme"),
                                                        QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                        Tr::tr("Keyboard Mapping Scheme (*.kms)"));
    if (filePath.isEmpty())
        return;

    QSettings settings(filePath, QSettings::IniFormat);
    for (auto item : qAsConst(shortcutItems)) {
        const auto &id = item->cmd->id();
        const auto settingsKey = QString(kKeyboardShortcuts) + '/' + id;
        const QList<QKeySequence> keys = item->shortcutKeys;
        if (keys.isEmpty()) {
            settings.setValue(settingsKey, QString());
        } else if (keys.size() == 1) {
            settings.setValue(settingsKey, keys.first().toString());
        } else {
            QStringList shortcutList;
            std::transform(keys.begin(), keys.end(), std::back_inserter(shortcutList),
                           [](const QKeySequence &k) {
                               return k.toString();
                           });
            settings.setValue(settingsKey, shortcutList);
        }
    }
}

void ShortcutSettingWidgetPrivate::importAction()
{
    const auto &filePath = QFileDialog::getOpenFileName(q, Tr::tr("Import Keyboard Mapping Scheme"),
                                                        QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                        Tr::tr("Keyboard Mapping Scheme (*.kms)"));
    if (filePath.isEmpty())
        return;

    // read shortcut from `filePath`
    QMap<QString, QList<QKeySequence>> mapping;
    QSettings settings(filePath, QSettings::IniFormat);
    settings.beginGroup(kKeyboardShortcuts);
    QStringList keys = settings.allKeys();
    for (const QString &key : keys) {
        const QVariant v = settings.value(key);
        if (QMetaType::Type(v.type()) == QMetaType::QStringList) {
            auto list = v.toStringList();
            QList<QKeySequence> keySequenceList;
            std::transform(list.begin(), list.end(), std::back_inserter(keySequenceList),
                           [](const QString &s) {
                               return QKeySequence::fromString(s);
                           });
            mapping.insert(key, keySequenceList);
        } else {
            mapping.insert(key, { QKeySequence::fromString(v.toString()) });
        }
    }
    settings.endGroup();

    // set shortcut
    for (ShortcutItem *item : std::as_const(shortcutItems)) {
        QString sid = item->cmd->id();
        if (mapping.contains(sid))
            updateShortcut(item->item, mapping.value(sid));
    }
}

void ShortcutSettingWidgetPrivate::handleKeyRecord()
{
    if (recordKeyBtn->isChecked()) {
        searchEdit->setPlaceholderText(Tr::tr("Recording Keys. Press Escape to exit"));
    } else {
        searchEdit->setPlaceholderText(Tr::tr("Type to search in keybindings"));
    }

    searchEdit->setFocus();
    searchEdit->lineEdit()->selectAll();
}

void ShortcutSettingWidgetPrivate::handleFilterChanged(const QString &f)
{
    if (f.isEmpty())
        resetRecordState();

    for (int i = 0; i < commandWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = commandWidget->topLevelItem(i);
        filter(f, item);
    }
}

bool ShortcutSettingWidgetPrivate::filter(const QString &filterString, QTreeWidgetItem *item)
{
    bool visible = filterString.isEmpty();
    int columnCount = item->columnCount();
    for (int i = 0; !visible && i < columnCount; ++i)
        visible |= !filterColumn(filterString, item, i);

    int childCount = item->childCount();
    if (childCount > 0) {
        // force visibility if this item matches
        QString tmpFilterString = visible ? QString() : filterString;
        for (int i = 0; i < childCount; ++i) {
            QTreeWidgetItem *citem = item->child(i);
            // parent visible if any child visible
            visible |= !filter(tmpFilterString, citem);
        }
    }
    item->setHidden(!visible);
    return !visible;
}

bool ShortcutSettingWidgetPrivate::filterColumn(const QString &filterString, QTreeWidgetItem *item, int column)
{
    const auto scItem = shortcutItem(item);
    // shortcut
    if (column == item->columnCount() - 1) {
        if (!scItem)
            return true;

        for (const auto &k : scItem->shortcutKeys) {
            const auto &keyString = k.toString(QKeySequence::PortableText);
            bool found = keyString.contains(filterString, Qt::CaseInsensitive);
            if (found)
                return false;
        }
        return true;
    }

    QString text;
    // command id
    if (column == 0 && scItem) {
        text = scItem->cmd->id();
    } else {
        text = item->text(column);
    }
    return !text.contains(filterString, Qt::CaseInsensitive);
}

ShortcutItem *ShortcutSettingWidgetPrivate::shortcutItem(QTreeWidgetItem *item)
{
    if (!item)
        return nullptr;
    return item->data(0, Qt::UserRole).value<ShortcutItem *>();
}

int ShortcutSettingWidgetPrivate::checkConflict(const QKeySequence &key, int index)
{
    if (key.isEmpty())
        return 0;

    QTreeWidgetItem *current = commandWidget->currentItem();
    ShortcutItem *item = shortcutItem(current);
    if (!item)
        return 0;

    int conflictCount = 0;
    for (ShortcutItem *currentItem : std::as_const(shortcutItems)) {
        if (item == currentItem)
            continue;

        for (const auto &k : qAsConst(currentItem->shortcutKeys)) {
            if (k == key)
                conflictCount++;
        }
    }

    return conflictCount;
}

void ShortcutSettingWidgetPrivate::setModified(QTreeWidgetItem *item, bool modified)
{
    QFont f = item->font(0);
    f.setItalic(modified);
    item->setFont(0, f);
    item->setFont(1, f);
    f.setBold(modified);
    item->setFont(2, f);
}

void ShortcutSettingWidgetPrivate::resetRecordState()
{
    keyNum = 0;
    searchEdit->clear();
    keyArray.fill(0);
}

int ShortcutSettingWidgetPrivate::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;
    if ((state & Qt::ShiftModifier)
        && (text.isEmpty()
            || !text.at(0).isPrint()
            || text.at(0).isLetterOrNumber()
            || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

QString ShortcutSettingWidgetPrivate::createPlaceholderString(const QStringList &list)
{
    if (list.isEmpty())
        return {};

    QFontMetrics metrics(q->font());
    QString placeholderString(20, ' ');
    for (int i = 0; i < list.size(); ++i) {
        if (i != 0)
            placeholderString.append(QString(22, ' '));

        placeholderString.append(QString(list[i].size(), ' '));
        int width = metrics.horizontalAdvance(list[i]);
        while (metrics.horizontalAdvance(placeholderString) < width) {
            placeholderString.append(' ');
        }
    }

    return placeholderString;
}

bool ShortcutSettingWidgetPrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (!searchEdit || !recordKeyBtn || (watched != searchEdit && watched != searchEdit->lineEdit()))
        return QObject::eventFilter(watched, event);

    if (recordKeyBtn->isChecked() && event->type() == QEvent::KeyPress) {
        auto k = static_cast<QKeyEvent *>(event);
        int nextKey = k->key();
        if (nextKey == Qt::Key_Escape) {
            recordKeyBtn->click();
            return true;
        }

        if (keyNum > 3)
            resetRecordState();

        if (nextKey == Qt::Key_Control
            || nextKey == Qt::Key_Shift
            || nextKey == Qt::Key_Meta
            || nextKey == Qt::Key_Alt) {
            return false;
        }

        nextKey |= translateModifiers(k->modifiers(), k->text());
        switch (keyNum) {
        case 0:
            keyArray[0] = nextKey;
            break;
        case 1:
            keyArray[1] = nextKey;
            break;
        case 2:
            keyArray[2] = nextKey;
            break;
        case 3:
            keyArray[3] = nextKey;
            break;
        default:
            break;
        }
        keyNum++;
        k->accept();
        QKeySequence shortcutKey(keyArray[0], keyArray[1], keyArray[2], keyArray[3]);
        searchEdit->setText(shortcutKey.toString(QKeySequence::NativeText));
        return true;
    }

    return QObject::eventFilter(watched, event);
}

ShortcutSettingWidget::ShortcutSettingWidget(QWidget *parent)
    : PageWidget(parent),
      d(new ShortcutSettingWidgetPrivate(this))
{
    d->initUI();
    d->inintConnection();
    d->updateCommands();
}

ShortcutSettingWidget::~ShortcutSettingWidget()
{
    delete d;
}

void ShortcutSettingWidget::readConfig()
{
    // Updated the shortcut key display name
    // For scenarios that are not saved after modification
    for (auto item : qAsConst(d->shortcutItems)) {
        if (item->shortcutKeys != item->cmd->keySequences())
            d->updateShortcut(item->item, item->cmd->keySequences());
    }
}

void ShortcutSettingWidget::saveConfig()
{
    d->apply();
    ActionManager::instance()->saveSettings();
}
