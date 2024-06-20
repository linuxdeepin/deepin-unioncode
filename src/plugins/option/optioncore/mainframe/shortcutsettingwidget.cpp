// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutsettingwidget.h"
#include "common/common.h"

#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DFileDialog>
#include <DKeySequenceEdit>
#include <DCommandLinkButton>
#include <DBackgroundGroup>

#include <QDebug>
#include <QDir>
#include <QAction>

DWIDGET_USE_NAMESPACE

class ShortCutPrivate
{
    QMap<QString, QStringList> shortcutItemMap;
    QMap<QString, QStringList> shortcutItemShadowMap;
    QString configFilePath;
    QVBoxLayout *vlayout = nullptr;
    QVBoxLayout *bgGplayout = nullptr;
    DBackgroundGroup *bgGroup = nullptr;

    friend class ShortCut;
};

ShortCut::ShortCut(QWidget *parent)
    : DFrame(parent), d(new ShortCutPrivate())
{
    d->configFilePath = (CustomPaths::user(CustomPaths::Flags::Configures)
                         + QDir::separator() + QString("shortcut.support"));
    readShortcut();
    setFrameShape(QFrame::NoFrame);

    d->vlayout = new QVBoxLayout(this);
    d->bgGplayout = new QVBoxLayout;
    d->bgGroup = new DBackgroundGroup(d->bgGplayout);
    d->bgGroup->setBackgroundRole(QPalette::Window);
    d->bgGroup->setUseWidgetBackground(false);
    d->vlayout->addWidget(d->bgGroup);
    updateUi();

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        updateDescriptions();
    });
}

ShortCut::~ShortCut()
{
}

void ShortCut::updateUi()
{
    //更新配置时先删除旧项
    if(d->bgGplayout->count()){
        while(auto layout = d->bgGplayout->takeAt(0))
            delete layout->widget();
    }

    QStringList ids = d->shortcutItemMap.keys();

    for (auto id : ids) {
        QStringList valueList = d->shortcutItemMap.value(id);
        QString description = valueList.first();
        QString shortcut = valueList.last();

        auto keyEdit = new DKeySequenceEdit(this);
        keyEdit->setKeySequence(QKeySequence(shortcut));
        keyEdit->setFixedWidth(300);
        keyEdit->ShortcutDirection(Qt::AlignLeft);
        keyEdit->setFocusPolicy(Qt::StrongFocus);

        auto label = new DLabel(description, this);
        auto wrapper = new DWidget;
        auto hlayout = new QHBoxLayout(wrapper);
        hlayout->setSpacing(0);
        hlayout->setContentsMargins(0, 0, 0, 0);
        hlayout->addWidget(label);
        hlayout->addWidget(keyEdit);
        hlayout->setAlignment(keyEdit, Qt::AlignRight);

        d->bgGplayout->addWidget(wrapper);
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
        if(d->shortcutItemMap[key].isEmpty())
            d->shortcutItemMap[key] = shortcutItemMap.value(key);
        else
            d->shortcutItemMap[key].last() = shortcutItemMap.value(key).last();
    }

    d->shortcutItemShadowMap = d->shortcutItemMap;
}

void ShortCut::updateDescriptions()
{
    QList<Command *> commandsList = ActionManager::getInstance()->commands();
    QList<Command *>::iterator iter = commandsList.begin();
    bool update = false;
    for (; iter != commandsList.end(); ++iter) {
        Action *action = dynamic_cast<Action *>(*iter);
        QString id = action->id();
        //已存在该Id的快捷键，但描述不同 -- 比如 配置中为中文 但应用为英文，显示会异常。
        if(!d->shortcutItemMap[id].isEmpty() && (d->shortcutItemMap[id].first() != action->description())) {
            d->shortcutItemMap[id].first() = action->description();
            update = true;
        }
    }

    if(update) {
        updateUi();
        d->shortcutItemShadowMap = d->shortcutItemMap;
    }
}

void ShortCut::importExternalJson(const QString &filePath)
{
    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(filePath, shortcutItemMap);
    foreach (QString key, shortcutItemMap.keys()) {
        d->shortcutItemMap[key] = shortcutItemMap.value(key);
    }

    d->shortcutItemShadowMap = d->shortcutItemMap;
    updateUi();
}

void ShortCut::exportExternalJson(const QString &filePath)
{
    ShortcutUtil::writeToJson(filePath, d->shortcutItemMap);
}

class ShortCut;
class ShortcutSettingWidgetPrivate
{
    ShortcutSettingWidgetPrivate();
    ShortCut *shortcut;

    friend class ShortcutSettingWidget;
};

ShortcutSettingWidgetPrivate::ShortcutSettingWidgetPrivate()
    : shortcut(nullptr)
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
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    auto shortcutFrame = new DFrame(this);
    auto shortcutLayout = new QVBoxLayout(shortcutFrame);

    d->shortcut = new ShortCut(shortcutFrame);

    shortcutLayout->addWidget(d->shortcut);

    vLayout->addWidget(shortcutFrame);

    DCommandLinkButton *resetbtn = new DCommandLinkButton(tr("Reset All"));
    DCommandLinkButton *exportbtn = new DCommandLinkButton(tr("Export"));
    DCommandLinkButton *importbtn = new DCommandLinkButton(tr("Import"));

    auto spliter = new DVerticalLine(this);

    auto hlayout = new QHBoxLayout;
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
    d->shortcut->updateUi();
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
