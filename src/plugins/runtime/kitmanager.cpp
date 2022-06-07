/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

static QString settingsFileName()
{
    return CustomPaths::global(CustomPaths::Configures) + KIT_FILENAME;
}

static KitManagerPrivate *d = nullptr;

KitManager::KitManager(QObject *parent) : QObject(parent)
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
    QTC_ASSERT(!d->initialized, return );

    std::vector<std::unique_ptr<Kit>> resultList;

    // read all kits from user file
    QString defaultUserKit;
    std::vector<std::unique_ptr<Kit>> kitsToCheck;
    {
        KitList userKits = restoreKits(settingsFileName());
        defaultUserKit = userKits.defaultKit;

        for (auto &k : userKits.kits) {
            resultList.emplace_back(std::move(k));
        }
    }

    // Delete all loaded autodetected kits that were not rediscovered:
    kitsToCheck.clear();

    if (resultList.size() == 0) {
        auto defaultKit = std::make_unique<Kit>(""); // One kit using default values
        defaultKit->setUnexpandedDisplayName(tr("Desktop"));

        resultList.emplace_back(std::move(defaultKit));
    }

    std::vector<std::unique_ptr<Kit>>::const_iterator begin = std::begin(resultList);
    std::vector<std::unique_ptr<Kit>>::const_iterator end = std::end(resultList);

    auto it = std::find_if(begin, end, equal(&Kit::id, defaultUserKit));

    Kit *k = it->get();
    if (!k)
        k = resultList.front().get();
    std::swap(resultList, d->kitList);
    setDefaultKit(k);

    d->writer = std::make_unique<PersistentSettingsWriter>(settingsFileName(), "QtCreatorProfiles");
    d->initialized = true;
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

QList<Kit *> KitManager::kits()
{
    // TODO(Mozart):
    return {};
}

Kit *KitManager::kit(QString id)
{
    // TODO(Mozart):
    return nullptr;
}

Kit *KitManager::defaultKit()
{
    // TODO(Mozart):
    return nullptr;
}

bool KitManager::registerKit(std::unique_ptr<Kit> &&k)
{
    // TODO(Mozart):
    return true;
}

void KitManager::deregisterKit(Kit *k)
{
    // TODO(Mozart)
}

void KitManager::setDefaultKit(Kit *k)
{
    // TODO(Mozart)
}

void KitManager::saveKits()
{
    // TODO(Mozart)
}

bool KitManager::isLoaded()
{
    // TODO(Mozart):
    return true;
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
