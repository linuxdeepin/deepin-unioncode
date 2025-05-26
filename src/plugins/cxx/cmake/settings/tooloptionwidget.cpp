// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_define.h"
#include "tooloptionwidget.h"

#include "services/option/toolchaindata.h"

#include <QDebug>

class ToolOptionWidgetPrivate : public QObject
{
public:
    explicit ToolOptionWidgetPrivate(ToolOptionWidget *qq);

    void initConnection();
    void initTools();

    QStandardItem *appendTool(const QString &name, const QString &path, OptionType type);
    void handleAddTool();

public:
    ToolOptionWidget *q;

    QStandardItem *detectedToolItem { nullptr };
    QStandardItem *manualToolItem { nullptr };
};

ToolOptionWidgetPrivate::ToolOptionWidgetPrivate(ToolOptionWidget *qq)
    : q(qq)
{
}

void ToolOptionWidgetPrivate::initConnection()
{
    connect(q, &ToolOptionWidget::requestAddOption, this, &ToolOptionWidgetPrivate::handleAddTool);
}

void ToolOptionWidgetPrivate::initTools()
{
    detectedToolItem = new QStandardItem(ToolOptionWidget::tr("Auto-detected"));
    manualToolItem = new QStandardItem(ToolOptionWidget::tr("Manual"));
    q->addItem(detectedToolItem);
    q->addItem(manualToolItem);

    ToolChainData toolCD;
    QString retMsg;
    bool ret = toolCD.readToolChainData(retMsg);
    if (!ret) {
        qWarning() << retMsg;
        return;
    }

    const auto &data = toolCD.getToolChanins();
    ToolChainData::Params params = data.value(kCCXXBuildSystems);
    for (const auto &param : params) {
        if (param.name.isEmpty() || param.path.isEmpty())
            continue;
        appendTool(param.name, param.path, Detected);
    }
}

QStandardItem *ToolOptionWidgetPrivate::appendTool(const QString &name, const QString &path, OptionType type)
{
    QString tmpName = name;
    if (name.isEmpty())
        tmpName = ToolOptionWidget::tr("New Tool");

    QStandardItem *item = new QStandardItem(Option::displayName(tmpName, path));
    item->setData(tmpName, OptionNameRole);
    item->setData(path, OptionPathRole);
    item->setData(type, OptionTypeRole);
    q->checkAndSetItemValid(item);

    switch (type) {
    case Detected:
        detectedToolItem->appendRow(item);
        break;
    case Manual:
        manualToolItem->appendRow(item);
        break;
    }

    return item;
}

void ToolOptionWidgetPrivate::handleAddTool()
{
    QStandardItem *item = appendTool(ToolOptionWidget::tr("New CMake"), "", Manual);
    if (item) {
        auto index = q->indexFromItem(item);
        q->setCurrentIndex(index);
    }
}

ToolOptionWidget::ToolOptionWidget(QWidget *parent)
    : BaseOptionWidgetHelper(parent),
      d(new ToolOptionWidgetPrivate(this))
{
    d->initConnection();
    d->initTools();
}

ToolOptionWidget::~ToolOptionWidget()
{
    delete d;
}

QPair<QString, QList<Option>> ToolOptionWidget::getOptions() const
{
    QList<Option> options;
    for (int i = 0; i < d->detectedToolItem->rowCount(); ++i) {
        Option opt;
        auto item = d->detectedToolItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();

        options.append(opt);
    }

    for (int i = 0; i < d->manualToolItem->rowCount(); ++i) {
        Option opt;
        auto item = d->manualToolItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();

        options.append(opt);
    }

    return qMakePair(QString(CMakeToolOptionName), options);
}

QString ToolOptionWidget::titleName() const
{
    return tr("CMake Tools");
}

QString ToolOptionWidget::configName() const
{
    return CMakeToolOptionName;
}

void ToolOptionWidget::setConfig(const QVariantMap &config)
{
    clearItem(d->manualToolItem);

    for (const auto &v : config) {
        const auto &map = v.toMap();
        const auto &name = map.value(Name).toString();
        if (name.isEmpty())
            continue;

        const auto &path = map.value(Path).toString();
        d->appendTool(name, path, Manual);
    }

    sort();
    expandAll();
}

QVariantMap ToolOptionWidget::getConfig() const
{
    applyCurrentOption();

    QVariantMap config;
    for (int i = 0; i < d->manualToolItem->rowCount(); ++i) {
        auto item = d->manualToolItem->child(i);
        const auto &name = item->data(OptionNameRole).toString();
        const auto &path = item->data(OptionPathRole).toString();

        QVariantMap map;
        map.insert(Name, name);
        map.insert(Path, path);
        config.insert(QString::number(i), map);
    }

    return config;
}
