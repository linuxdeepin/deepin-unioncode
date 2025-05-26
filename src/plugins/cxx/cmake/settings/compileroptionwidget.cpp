// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_define.h"
#include "compileroptionwidget.h"

#include "services/option/toolchaindata.h"

#include <QMenu>
#include <QDebug>

inline constexpr char Type[] { "type" };

class CompilerOptionWidgetPrivate : public QObject
{
public:
    enum CompilerType {
        DetectedCCompiler = 0,
        DetectedCXXCompiler,
        ManualCCompiler,
        ManualCXXCompiler
    };

    explicit CompilerOptionWidgetPrivate(CompilerOptionWidget *qq);

    void initCompilers();

    QStandardItem *appendCompiler(const QString &name, const QString &path, CompilerType type);
    void handleAddCompiler(CompilerType type);

public:
    CompilerOptionWidget *q;

    QStandardItem *detectedCItem { nullptr };
    QStandardItem *detectedCXXItem { nullptr };
    QStandardItem *manualCItem { nullptr };
    QStandardItem *manualCXXItem { nullptr };
};

CompilerOptionWidgetPrivate::CompilerOptionWidgetPrivate(CompilerOptionWidget *qq)
    : q(qq)
{
    QMenu *menu = new QMenu(q);
    menu->addAction("C", this, std::bind(&CompilerOptionWidgetPrivate::handleAddCompiler, this, ManualCCompiler));
    menu->addAction("C++", this, std::bind(&CompilerOptionWidgetPrivate::handleAddCompiler, this, ManualCXXCompiler));
    q->setAddButtonMenu(menu);
}

void CompilerOptionWidgetPrivate::initCompilers()
{
    QStandardItem *detectedItem = new QStandardItem(CompilerOptionWidget::tr("Auto-detected"));
    detectedItem->appendRow(detectedCItem = new QStandardItem("C"));
    detectedItem->appendRow(detectedCXXItem = new QStandardItem("C++"));

    QStandardItem *manualItem = new QStandardItem(CompilerOptionWidget::tr("Manual"));
    manualItem->appendRow(manualCItem = new QStandardItem("C"));
    manualItem->appendRow(manualCXXItem = new QStandardItem("C++"));
    q->addItem(detectedItem);
    q->addItem(manualItem);

    ToolChainData toolCD;
    QString retMsg;
    bool ret = toolCD.readToolChainData(retMsg);
    if (!ret) {
        qWarning() << retMsg;
        return;
    }

    const auto &data = toolCD.getToolChanins();
    ToolChainData::Params cParams = data.value(kCCompilers);
    for (const auto &param : cParams) {
        if (param.name.isEmpty() || param.path.isEmpty())
            continue;
        appendCompiler(param.name, param.path, DetectedCCompiler);
    }

    ToolChainData::Params cxxParams = data.value(kCXXCompilers);
    for (const auto &param : cxxParams) {
        if (param.name.isEmpty() || param.path.isEmpty())
            continue;
        appendCompiler(param.name, param.path, DetectedCXXCompiler);
    }
}

QStandardItem *CompilerOptionWidgetPrivate::appendCompiler(const QString &name, const QString &path, CompilerType type)
{
    QString tmpName = name;
    if (name.isEmpty())
        tmpName = type % 2
                ? CompilerOptionWidget::tr("New C++ Compiler")
                : CompilerOptionWidget::tr("New C Compiler");

    QStandardItem *item = new QStandardItem(Option::displayName(tmpName, path));
    item->setData(tmpName, OptionNameRole);
    item->setData(path, OptionPathRole);
    q->checkAndSetItemValid(item);

    switch (type) {
    case DetectedCCompiler:
        item->setData(Detected, OptionTypeRole);
        detectedCItem->appendRow(item);
        break;
    case DetectedCXXCompiler:
        item->setData(Detected, OptionTypeRole);
        detectedCXXItem->appendRow(item);
        break;
    case ManualCCompiler:
        item->setData(Manual, OptionTypeRole);
        manualCItem->appendRow(item);
        break;
    case ManualCXXCompiler:
        item->setData(Manual, OptionTypeRole);
        manualCXXItem->appendRow(item);
        break;
    }

    return item;
}

void CompilerOptionWidgetPrivate::handleAddCompiler(CompilerType type)
{
    QStandardItem *item { nullptr };
    switch (type) {
    case ManualCCompiler:
        item = appendCompiler(CompilerOptionWidget::tr("New C Compiler"), "", type);
        break;
    case ManualCXXCompiler:
        item = appendCompiler(CompilerOptionWidget::tr("New C++ Compiler"), "", type);
        break;
    default:
        break;
    }

    if (item) {
        auto index = q->indexFromItem(item);
        q->setCurrentIndex(index);
    }
}

CompilerOptionWidget::CompilerOptionWidget(QWidget *parent)
    : BaseOptionWidgetHelper(parent),
      d(new CompilerOptionWidgetPrivate(this))
{
    d->initCompilers();
}

CompilerOptionWidget::~CompilerOptionWidget()
{
    delete d;
}

QPair<QString, QList<Option> > CompilerOptionWidget::getOptions() const
{
    QList<Option> options;
    for (int i = 0; i < d->detectedCItem->rowCount(); ++i) {
        Option opt;
        auto item = d->detectedCItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();
        opt.userData = kCCompiler;

        options.append(opt);
    }

    for (int i = 0; i < d->detectedCXXItem->rowCount(); ++i) {
        Option opt;
        auto item = d->detectedCXXItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();
        opt.userData = kCXXCompiler;

        options.append(opt);
    }

    for (int i = 0; i < d->manualCItem->rowCount(); ++i) {
        Option opt;
        auto item = d->manualCItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();
        opt.userData = kCCompiler;

        options.append(opt);
    }

    for (int i = 0; i < d->manualCXXItem->rowCount(); ++i) {
        Option opt;
        auto item = d->manualCXXItem->child(i);
        opt.name = item->data(OptionNameRole).toString();
        opt.path = item->data(OptionPathRole).toString();
        opt.userData = kCXXCompiler;

        options.append(opt);
    }

    return qMakePair(QString(CompilerOptionName), options);
}

QString CompilerOptionWidget::titleName() const
{
    return tr("Compilers");
}

QString CompilerOptionWidget::configName() const
{
    return CompilerOptionName;
}

void CompilerOptionWidget::setConfig(const QVariantMap &config)
{
    clearItem(d->manualCItem);
    clearItem(d->manualCXXItem);

    for (const auto &v : config) {
        const auto &map = v.toMap();
        const auto &name = map.value(Name).toString();
        if (name.isEmpty())
            continue;

        const auto &type = map.value(Type).toInt();
        const auto &path = map.value(Path).toString();
        d->appendCompiler(name, path, static_cast<CompilerOptionWidgetPrivate::CompilerType>(type));
    }

    sort();
    expandAll();
}

QVariantMap CompilerOptionWidget::getConfig() const
{
    applyCurrentOption();

    QVariantMap config;
    int index = 0;
    for (int i = 0; i < d->manualCItem->rowCount(); ++i) {
        auto item = d->manualCItem->child(i);
        const auto &name = item->data(OptionNameRole).toString();
        const auto &path = item->data(OptionPathRole).toString();

        QVariantMap map;
        map.insert(Type, CompilerOptionWidgetPrivate::ManualCCompiler);
        map.insert(Name, name);
        map.insert(Path, path);
        config.insert(QString::number(index++), map);
    }

    for (int i = 0; i < d->manualCXXItem->rowCount(); ++i) {
        auto item = d->manualCXXItem->child(i);
        const auto &name = item->data(OptionNameRole).toString();
        const auto &path = item->data(OptionPathRole).toString();

        QVariantMap map;
        map.insert(Type, CompilerOptionWidgetPrivate::ManualCXXCompiler);
        map.insert(Name, name);
        map.insert(Path, path);
        config.insert(QString::number(index++), map);
    }

    return config;
}
