/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<huangyub@uniontech.com>
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
#ifndef QTCASSERT_H
#define QTCASSERT_H

//namespace Utils { void writeAssertLocation(const char *msg){} }

#define QTC_ASSERT_STRINGIFY_HELPER(x) #x
#define QTC_ASSERT_STRINGIFY(x) QTC_ASSERT_STRINGIFY_HELPER(x)
#define QTC_ASSERT_STRING(cond) /*::Utils::writeAssertLocation(\
    "\"" cond"\" in file " __FILE__ ", line " QTC_ASSERT_STRINGIFY(__LINE__))*/

// The 'do {...} while (0)' idiom is not used for the main block here to be
// able to use 'break' and 'continue' as 'actions'.

#define QTC_ASSERT(cond, action) if (cond) {} else { QTC_ASSERT_STRING(#cond); action; } do {} while (0)
#define QTC_CHECK(cond) if (cond) {} else { QTC_ASSERT_STRING(#cond); } do {} while (0)
#define QTC_GUARD(cond) ((cond) ? true : (QTC_ASSERT_STRING(#cond), false))

#endif // QTCASSERT_H
