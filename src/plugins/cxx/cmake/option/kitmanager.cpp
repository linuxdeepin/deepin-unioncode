// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitmanager.h"
#include "common/util/custompaths.h"
#include "common/util/qtcassert.h"
#include "persistentsettings.h"

#include <QFileInfo>

const char KIT_DATA_KEY[] = "Profile.";
const char KIT_COUNT_KEY[] = "Profile.Count";
const char KIT_DEFAULT_KEY[] = "Profile.Default";
const char KIT_FILENAME[] = "/profiles.xml";

class KitManagerPrivate
{
public:
    Kit *defaultKit = nullptr;
    bool initialized = false;
    std::vector<std::unique_ptr<Kit>> kitList;
    Kit selectedKit;
    std::unique_ptr<PersistentSettingsWriter> writer;
};

static KitManagerPrivate *d = nullptr;

KitManager::KitManager(QObject *parent)
    : QObject(parent)
{
    d = new KitManagerPrivate;
}

//////////////////
// find helpers
//////////////////
template<typename R, typename S, typename T>
decltype(auto) equal(R (S::*function)() const, T value)
{
    // This should use std::equal_to<> instead of std::equal_to<T>,
    // but that's not supported everywhere yet, since it is C++14
    return std::bind<bool>(std::equal_to<T>(), value, std::bind(function, std::placeholders::_1));
}

void KitManager::restoreKits()
{
    // TODO(Mozart)
}

KitManager::KitList KitManager::restoreKits(const QString &fileName)
{
    KitList result;

    QFileInfo info(fileName);
    if (!info.exists())
        return result;

    PersistentSettingsReader reader;
    if (!reader.load(fileName)) {
        qWarning("Warning: Failed to read \"%s\", cannot restore kits!",
                 qPrintable(fileName));
        return result;
    }
    QVariantMap data = reader.restoreValues();

    const int count = data.value(QLatin1String(KIT_COUNT_KEY), 0).toInt();
    for (int i = 0; i < count; ++i) {
        const QString key = QString::fromLatin1(KIT_DATA_KEY) + QString::number(i);
        if (!data.contains(key))
            break;

        const QVariantMap stMap = data.value(key).toMap();

        auto k = std::make_unique<Kit>(stMap);

        result.kits.emplace_back(std::move(k));
    }
    const QString id = data.value(QLatin1String(KIT_DEFAULT_KEY)).toString();
    if (id.isEmpty())
        return result;

    std::vector<std::unique_ptr<Kit>>::iterator it;
    for (; it != result.kits.end(); ++it) {
        if (it->get()->id() == id) {
            result.defaultKit = id;
            break;
        }
    }

    return result;
}

KitManager *KitManager::instance()
{
    static KitManager ins;
    return &ins;
}

KitManager::~KitManager()
{
}

void KitManager::setSelectedKit(Kit &kit)
{
    d->selectedKit = kit;
}

const Kit &KitManager::getSelectedKit()
{
    return d->selectedKit;
}

QString KitManager::getDefaultOutputPath() const
{
    return d->selectedKit.getDefaultOutput();
}
