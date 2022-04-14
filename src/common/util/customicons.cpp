#include "customicons.h"
#include <QIcon>
#include <QMimeDatabase>

namespace {
QFileIconProvider provider;
QMimeDatabase mimeDatabase;
}

QIcon CustomIcons::icon(QFileIconProvider::IconType type)
{
    return provider.icon(type);
}

QIcon CustomIcons::icon(CustomIcons::CustomIconType type)
{
    switch (type) {
    case CustomIcons::Exe:
        return QIcon::fromTheme("application/x-executable");
    case CustomIcons::Lib:
        return QIcon::fromTheme("application/x-sharedlib");
    }
}

QIcon CustomIcons::icon(const QFileInfo &info)
{
    return provider.icon(info);
}

QString CustomIcons::type(const QFileInfo &info)
{
    return provider.type(info);
}

void CustomIcons::setOptions(QFileIconProvider::Options options)
{
    return provider.setOptions(options);
}

QFileIconProvider::Options CustomIcons::options()
{
    return provider.options();
}
