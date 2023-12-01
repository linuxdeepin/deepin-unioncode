// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutsettingwidget.h"
#include "common/common.h"

#include <DTableView>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DHeaderView>
#include <DFileDialog>
#include <DKeySequenceEdit>
#include <DCommandLinkButton>

#include <QFormLayout>
#include <QDebug>
#include <QDir>
#include <QAction>

#define BTN_WIDTH (180)

DWIDGET_USE_NAMESPACE

class ShortCutPrivate
{
    QMap<QString, QStringList> shortcutItemMap;
    QMap<QString, QStringList> shortcutItemShadowMap;
    QString configFilePath;
    QFormLayout *formlayout = nullptr;

    friend class ShortCut;
};

ShortCut::ShortCut(QWidget *parent)
    : DFrame(parent), d(new ShortCutPrivate())
{
    d->configFilePath = (CustomPaths::user(CustomPaths::Flags::Configures)
                         + QDir::separator() + QString("shortcut.support"));
    readShortcut();
    setFrameShape(QFrame::NoFrame);
    d->formlayout = new QFormLayout(this);
    setupForm();
}

ShortCut::~ShortCut()
{
}

void ShortCut::setupForm()
{
    //在重置配置时先删除旧项
    if (d->formlayout)
        while (d->formlayout->rowCount())
            d->formlayout->removeRow(0);

    QStringList ids = d->shortcutItemMap.keys();

    for (auto id : ids) {
        QStringList valueList = d->shortcutItemMap.value(id);
        QString description = valueList.first();
        QString shortcut = valueList.last();

        auto keyEdit = new DKeySequenceEdit(this);
        keyEdit->setKeySequence(QKeySequence(shortcut));
        keyEdit->setText(QAction::tr(description.toStdString().c_str()));
        d->formlayout->addRow(keyEdit);

        connect(keyEdit, &DKeySequenceEdit::editingFinished, this, [=](const QKeySequence &sequence) {
            bool inValid = keySequenceIsInvalid(sequence);
            QString oldShortcut = d->shortcutItemMap.value(id).last();
            //todo(zta):
                //异常情况下的警告：文本编辑器使用的弹窗，这里以前使用的是label来输出警告文字，但是现在整个窗口太大，警告文字会在下方，大部分情况下看不到。
            if (inValid) {
                qWarning() << "keySequence is invalid";
                keyEdit->setKeySequence(QKeySequence(oldShortcut));
                return;
            }
            bool isRepeat = shortcutRepeat(sequence.toString());
            if (isRepeat) {
                qWarning() << "isRepeat!!";
                keyEdit->setKeySequence(QKeySequence(oldShortcut));
                return;
            }
            updateShortcut(id, sequence.toString());
        });
    }
}

int ShortCut::rowCount() const
{
    return d->shortcutItemMap.size();
}

bool ShortCut::keySequenceIsInvalid(const QKeySequence &sequence) const
{
    for (uint i = 0; i < static_cast<uint>(sequence.count()); i++) {
        if (Qt::Key_unknown == sequence[i]) {
            return true;
        }
    }

    return false;
}

bool ShortCut::shortcutRepeat(const QString &text) const
{
    int count = 0;
    foreach (QString key, d->shortcutItemMap.keys()) {
        QStringList valueList = d->shortcutItemMap.value(key);
        if (0 == text.compare(valueList.last(), Qt::CaseInsensitive)) {
            if (++count > 0)
                return true;
        }
    }

    return false;
}

void ShortCut::updateShortcut(QString id, QString shortcut)
{
    if (d->shortcutItemMap.keys().contains(id)) {
        QStringList valueList = d->shortcutItemMap.value(id);
        QStringList newValueList = { valueList.first(), shortcut };
        d->shortcutItemMap[id] = newValueList;
    }
}

void ShortCut::resetAllShortcut()
{
    d->shortcutItemMap = d->shortcutItemShadowMap;
}

void ShortCut::saveShortcut()
{
    ShortcutUtil::writeToJson(d->configFilePath, d->shortcutItemMap);

    QList<Command *> commandsList = ActionManager::getInstance()->commands();
    QList<Command *>::iterator iter = commandsList.begin();
    for (; iter != commandsList.end(); ++iter) {
        Action *action = dynamic_cast<Action *>(*iter);
        QString id = action->id();

        if (d->shortcutItemMap.contains(id)) {
            QStringList valueList = d->shortcutItemMap[id];
            action->setKeySequence(QKeySequence(valueList.last()));
        }
    }
}

void ShortCut::readShortcut()
{
    QList<Command *> commandsList = ActionManager::getInstance()->commands();
    QList<Command *>::iterator iter = commandsList.begin();
    for (; iter != commandsList.end(); ++iter) {
        Action *action = dynamic_cast<Action *>(*iter);
        QString id = action->id();
        QStringList valueList = QStringList { action->description(), action->keySequence().toString() };
        d->shortcutItemMap[id] = valueList;
    }

    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(d->configFilePath, shortcutItemMap);
    foreach (const QString key, shortcutItemMap.keys()) {
        d->shortcutItemMap[key] = shortcutItemMap.value(key);
    }

    d->shortcutItemShadowMap = d->shortcutItemMap;
}

void ShortCut::importExternalJson(const QString &filePath)
{
    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(filePath, shortcutItemMap);
    foreach (QString key, shortcutItemMap.keys()) {
        d->shortcutItemMap[key] = shortcutItemMap.value(key);
    }

    d->shortcutItemShadowMap = d->shortcutItemMap;
    setupForm();
}

void ShortCut::exportExternalJson(const QString &filePath)
{
    ShortcutUtil::writeToJson(filePath, d->shortcutItemMap);
}

class ShortCut;
class ShortcutSettingWidgetPrivate
{
    ShortcutSettingWidgetPrivate();
    DTableView *tableView;
    HotkeyLineEdit *editShortCut;
    ShortCut *shortcut;
    DPushButton *btnRecord;
    DLabel *tipLabel;

    friend class ShortcutSettingWidget;
};

ShortcutSettingWidgetPrivate::ShortcutSettingWidgetPrivate()
    : tableView(nullptr), editShortCut(nullptr), shortcut(nullptr), btnRecord(nullptr), tipLabel(nullptr)
{
}

ShortcutSettingWidget::ShortcutSettingWidget(QWidget *parent)
    : PageWidget(parent), d(new ShortcutSettingWidgetPrivate())
{
    setupUi();
    readConfig();
}

ShortcutSettingWidget::~ShortcutSettingWidget()
{
}

void ShortcutSettingWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    auto tableFrame = new DFrame(this);
    auto tableLayout = new QVBoxLayout(tableFrame);
    tableFrame->setLayout(tableLayout);

    d->shortcut = new ShortCut(tableFrame);

    tableLayout->addWidget(d->shortcut);

    vLayout->addWidget(tableFrame);

    DCommandLinkButton *resetbtn = new DCommandLinkButton(tr("Reset All"));

    auto spliter = new DVerticalLine(this);
    DCommandLinkButton *exportbtn = new DCommandLinkButton(tr("Export"));
    DCommandLinkButton *importbtn = new DCommandLinkButton(tr("Import"));

    auto hlayout = new QHBoxLayout(this);
    hlayout->setAlignment(Qt::AlignRight);
    hlayout->addWidget(resetbtn);
    hlayout->addWidget(spliter);
    hlayout->addWidget(exportbtn);
    hlayout->addWidget(importbtn);

    vLayout->addLayout(hlayout);

    connect(resetbtn, &DCommandLinkButton::clicked, this, &ShortcutSettingWidget::onBtnResetAllClicked);
    connect(exportbtn, &DCommandLinkButton::clicked, this, &ShortcutSettingWidget::onBtnExportClicked);
    connect(importbtn, &DCommandLinkButton::clicked, this, &ShortcutSettingWidget::onBtnImportClicked);
}

void ShortcutSettingWidget::onBtnResetAllClicked()
{
    d->shortcut->resetAllShortcut();
    d->shortcut->setupForm();
}

void ShortcutSettingWidget::saveConfig()
{
    d->shortcut->saveShortcut();
}

void ShortcutSettingWidget::readConfig()
{
    d->shortcut->readShortcut();
}

void ShortcutSettingWidget::onBtnImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), tr(""), tr("Json File(*.json)"));
    if (!fileName.isEmpty()) {
        d->shortcut->importExternalJson(fileName);
    }
}

void ShortcutSettingWidget::onBtnExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), tr(""), tr("Json File(*.json)"));
    if (!fileName.isEmpty()) {
        d->shortcut->exportExternalJson(fileName);
    }
}
