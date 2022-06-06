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
#ifndef SYMBOLINFO_H
#define SYMBOLINFO_H

#include <QVariantHash>
#include <QStandardItem>
#include <QSet>

namespace libClang{
class Location
{
public:
    Location(const QString &filePath, int line, int column, int offset)
        : filePath(filePath), line(line), column(column), offset(offset){}

    Location(const Location &other)
        : filePath(other.filePath), line(other.line),
          column(other.column), offset(other.offset){}

    Location() : filePath(""), line(-1), column(-1), offset(-1){}

    inline QString getFilePath() const { return filePath; }
    inline int getLine() const { return line; }
    inline int getColumn() const { return column; }
    inline int getOffset() const { return offset; }
    inline void setFilePath(const QString filePath) { Location::filePath = filePath; }
    inline void setLine(int line) { Location::line = line; }
    inline void setColumn(int column) { Location::column = column; }
    inline void setOffset(int offset) { Location::offset = offset; }
    inline Location& operator = (const Location &other) {
        filePath = other.filePath;
        line = other.line;
        column = other.column;
        offset = other.offset;
        return *this;
    }

    inline bool operator == (const Location &other) {
        return filePath == other.filePath
                && line == other.line
                && column == other.column
                && offset == other.offset;
    }

    inline bool isValid () const {
        return !filePath.isEmpty() && line != -1
                && column != -1 && offset != -1;
    }

    inline bool isEmpty() const {
        return filePath.isEmpty() && line == -1
                && column == -1
                && offset == -1;
    }
protected:
    QString filePath;
    int line;
    int column;
    int offset;
};

inline bool operator == (const libClang::Location &v1, const libClang::Location &v2)
{
    return v1.getFilePath() == v2.getFilePath()
            && v1.getLine() == v2.getLine()
            && v1.getColumn() == v2.getColumn()
            && v1.getOffset() == v2.getOffset();
}

class KindInfo
{
public:
    KindInfo(const QString &kind, const QString &type, const QString &value)
        : kind(kind), type(type), value(value){}
    KindInfo(const KindInfo &kindInfo)
        : kind(kindInfo.kind), type(kindInfo.type), value(kindInfo.value){}

    KindInfo() = default;
    inline QString getKind() const { return kind; }
    inline QString getType() const { return type; }
    inline QString getValue() const { return value; }
    inline void setKind(const QString &kind) { KindInfo::kind = kind; }
    inline void setType(const QString &type) { KindInfo::type = type; }
    inline void setValue(const QString &value) {KindInfo::value = value; }
    inline KindInfo& operator = (const KindInfo &kindInfo) {
        kind = kindInfo.kind;
        type = kindInfo.type;
        value = kindInfo.value;
        return *this;
    }

    inline bool operator == (const KindInfo& kindInfo) {
        return kind == kindInfo.kind && type == kindInfo.type
                && value == kindInfo.value;
    }

    inline bool isValid () const {
        return !kind.isEmpty() && !type.isEmpty() && !value.isEmpty();
    }

    inline bool isEmpty() const {
        return kind.isEmpty() && type.isEmpty() && value.isEmpty();
    }

protected:
    QString kind;
    QString type;
    QString value;
};

class KindInfoLocations : public KindInfo, public QSet<Location>
{
public:
    KindInfoLocations() : KindInfo() , QSet<Location>() {}

    KindInfoLocations(const KindInfo &kindInfo, const QSet<Location> &locations)
        : KindInfo(kindInfo) , QSet<Location>(locations) {}

    KindInfoLocations(const KindInfoLocations &other)
        : KindInfo(other.getKindInfo()), QSet<Location>(other.getLocations()){}

    inline QSet<Location> getLocations() const { return *this; }
    inline KindInfo getKindInfo() const { return *this; }
    inline void setLocations(const QSet<Location> &locations)
    { QSet<Location>::operator = (locations); }

    inline auto operator << (const Location &location)
    { return QSet<Location>::operator<< (location); }

    inline KindInfoLocations & operator = (const KindInfoLocations &other) {
        KindInfo::operator=(other.getKindInfo());
        QSet<Location>::operator=(other.getLocations());
        return *this;
    }

    inline bool operator == (const KindInfoLocations &other) {
        return KindInfo::operator==(other.getKindInfo());
    }

    bool isEmpty() const { return KindInfo::isEmpty() && QSet<Location>::isEmpty(); }
    bool isValid() const { return KindInfo::isValid() && !QSet<Location>::isEmpty(); }
};

inline uint qHash(const libClang::Location &key, uint seed = 0)
{
    QString hashStr;
    hashStr += key.getFilePath();
    hashStr += " " + QString::number(key.getLine());
    hashStr += " " + QString::number(key.getColumn());
    hashStr += " " + QString::number(key.getOffset());
    return qHash(hashStr, seed);
}

} // namespace libClang

namespace dpfservice {

enum {
    SymbolInfoRole = Qt::ItemDataRole::UserRole + 1,
};

class SymbolInfo : public libClang::KindInfoLocations
{
public:
    SymbolInfo() {}

    SymbolInfo(const SymbolInfo &other)
        : libClang::KindInfoLocations(other) {}

    SymbolInfo(const libClang::KindInfoLocations &other)
        : libClang::KindInfoLocations(other){}

    SymbolInfo(const libClang::KindInfo &kindInfo,
               const QSet<libClang::Location> &location)
        : libClang::KindInfoLocations(kindInfo, location){}

    inline static SymbolInfo get(const QStandardItem *any) {
        if (!any)
            return {};
        using namespace libClang;
        QVariant var = any->data(SymbolInfoRole);
        if (var.canConvert<SymbolInfo>()) {
            return qvariant_cast<SymbolInfo>(var);
        }
        return {};
    }

    inline static SymbolInfo get(const QModelIndex &index) {
        if (!index.isValid())
            return {};
        using namespace libClang;
        QVariant var = index.data(SymbolInfoRole);
        if (var.canConvert<SymbolInfo>()) {
            return qvariant_cast<SymbolInfo>(var);
        }
        return {};
    }

    // clear && setting
    inline static void set(QStandardItem *any, const SymbolInfo &info) {
        if (!any)
            return;
        any->setData(QVariant::fromValue(info), SymbolInfoRole);
    }

    // shuold KindInfo is same, can to merge
    inline static void merge(QStandardItem *any, const SymbolInfo &info) {
        if (!any)
            return;
        auto symInfo = SymbolInfo::get(any);
        if (symInfo.getKindInfo() == info.getKindInfo()) {
           SymbolInfo result{symInfo.getKindInfo(), symInfo.getLocations() + info.getLocations()};
           SymbolInfo::set(any, result);
        }
    }
};
} // namespace dpfservice

Q_DECLARE_METATYPE(libClang::KindInfoLocations);
Q_DECLARE_METATYPE(dpfservice::SymbolInfo);

#endif // SYMBOLINFO_H
