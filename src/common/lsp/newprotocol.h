/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef NEWPROTOCOL_H
#define NEWPROTOCOL_H

#include "new/documentsynchronization.h"
#include "new/lifecyclemessage.h"
#include "new/basicjsonstructures.h"
//namespace newlsp {
//struct MarkedString : std::string
//{
//    std::string language;
//    std::string value;
//    MarkedString(const std::string &value) : std::string(value){}
//    MarkedString(const std::string &language, const std::string &value)
//        : std::string(), language(language), value(value) {}
//    MarkedString(const MarkedString &other)
//        : std::string(other) , language(other.language), value(other.value) {}
//};

//typedef QVector<MarkedString> MarkedStrings;

//struct Hover
//{
//    std::any contents; // MarkedString | MarkedStrings | MarkupContent
//    std::optional<Range> range;
//};
//} // newlsp
#endif // NEWPROTOCOL_H
