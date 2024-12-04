// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONDATASTRUCT_H
#define OPTIONDATASTRUCT_H

#include "toolchaindata.h"

#include <QObject>

namespace option {
static const QString CATEGORY_CMAKE{"CMake"};
static const QString CATEGORY_MAVEN{"Maven"};
static const QString CATEGORY_GRADLE{"Gradle"};
static const QString CATEGORY_JAVA{"Java"};
static const QString CATEGORY_PYTHON{"Python"};
static const QString CATEGORY_JS{"JS"};
static const QString CATEGORY_NINJA{"Ninja"};

static const QString GROUP_GENERAL{QObject::tr("General")};
static const QString GROUP_LANGUAGE{QObject::tr("Language")};
static const QString GROUP_AI{QObject::tr("AI")};
}
#endif // OPTIONDATASTRUCT_H
