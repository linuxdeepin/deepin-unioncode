// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_define.h"
#include "debuggeroptionwidget.h"

#include "services/option/toolchaindata.h"

#include <QDebug>

class DebuggerOptionWidgetPrivate : public QObject
{
public:
    explicit DebuggerOptionWidgetPrivate(DebuggerOptionWidget *qq);

    void initConnection();
    void initDebuggers();

    QStandardItem *appendDebugger(const QString &name, const QString &path, OptionType type);
    void handleAddDebugger();

public:
    DebuggerOptionWidget *q;

    QStandardItem *detectedDebuggerItem { nullptr };
    QStandardItem *manualDebuggerItem { nullptr };
};

DebuggerOptionWidgetPrivate::DebuggerOptionWidgetPrivate(DebuggerOptionWidget *qq)
    : q(qq)
{
}

void DebuggerOptionWidgetPrivate::initConnection()
{
    connect(q, &DebuggerOptionWidget::requestAddOption, this, &DebuggerOptionWidgetPrivate::handleAddDebugger);
}

void DebuggerOptionWidgetPrivate::initDebuggers()
{
    detectedDebuggerItem = new QStandardItem(DebuggerOptionWidget::tr("Auto-detected"));
    manualDebuggerItem = new QStandardItem(DebuggerOptionWidget::tr("Manual"));
    q->addItem(detectedDebuggerItem);
    q->addItem(manualDebuggerItem);

    ToolChainData toolCD;
    QString retMsg;
    bool ret = toolCD.readToolChainData(retMsg);
    if (!ret) {
        qWarning() << retMsg;
        return;
    }

    const auto &data = toolCD.getToolChanins();
    ToolChainData::Params params = data.value(kCCXXDebuggers);
    for (const auto &param : params) {
        appendDebugger(param.name, param.path, Detected);
    }
}

QStandardItem *DebuggerOptionWidgetPrivate::appendDebugger(const QString &name, const QString &path, OptionType type)
{
    QString tmpName = name;
    if (name.isEmpty())
        tmpName = DebuggerOptionWidget::tr("New Debugger");

    QStandardItem *item = new QStandardItem(Option::displayName(tmpName, path));
    item->setData(tmpName, OptionNameRole);
    item->setData(path, OptionPathRole);
    item->setData(type, OptionTypeRole);
    q->checkAndSetItemValid(item);

    switch (type) {
    case Detected:
        detectedDebuggerItem->appendRow(item);
        break;
    case Manual:
        manualDebuggerItem->appendRow(item);
        break;
    }

    return item;
}

void DebuggerOptionWidgetPrivate::handleAddDebugger()
{
    QStandardItem *item = appendDebugger(DebuggerOptionWidget::tr("New Debugger"), "", Manual);
    if (item) {
        auto index = q->indexFromItem(item);
        q->setCurrentIndex(index);
    }
}

DebuggerOptionWidget::DebuggerOptionWidget(QWidget *parent)
    : BaseOptionWidgetHelper(parent),
      d(new DebuggerOptionWidgetPrivate(this))
{
    d->initConnection();
    d->initDebuggers();
}

DebuggerOptionWidget::~DebuggerOptionWidget()
{
    delete d;
}

QPair<QString, QList<Option>> DebuggerOptionWidget::getOptions() const
{
    QList<Option> options;
    for (int i = 0; i < d->detectedDebuggerItem->rowCount(); ++i) {
        Option opt;
        auto item = d->detectedDebuggerItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();

        options.append(opt);
    }

    for (int i = 0; i < d->manualDebuggerItem->rowCount(); ++i) {
        Option opt;
        auto item = d->manualDebuggerItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();

        options.append(opt);
    }

    return qMakePair(QString(DebuggerOptionName), options);
}

QString DebuggerOptionWidget::titleName() const
{
    return tr("Debuggers");
}

QString DebuggerOptionWidget::configName() const
{
    return DebuggerOptionName;
}

void DebuggerOptionWidget::setConfig(const QVariantMap &config)
{
    clearItem(d->manualDebuggerItem);

    for (const auto &v : config) {
        const auto &map = v.toMap();
        const auto &name = map.value(Name).toString();
        if (name.isEmpty())
            continue;

        const auto &path = map.value(Path).toString();
        d->appendDebugger(name, path, Manual);
    }

    sort();
    expandAll();
}

QVariantMap DebuggerOptionWidget::getConfig() const
{
    applyCurrentOption();

    QVariantMap config;
    for (int i = 0; i < d->manualDebuggerItem->rowCount(); ++i) {
        auto item = d->manualDebuggerItem->child(i);
        const auto &name = item->data(OptionNameRole).toString();
        const auto &path = item->data(OptionPathRole).toString();

        QVariantMap map;
        map.insert(Name, name);
        map.insert(Path, path);
        config.insert(QString::number(i), map);
    }

    return config;
}
