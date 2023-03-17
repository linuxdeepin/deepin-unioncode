/*
 * Copyright (C) 2020 ~ 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#ifndef XMLSTREAMREADER_H
#define XMLSTREAMREADER_H

#include <QObject>
#include <QXmlStreamReader>

struct Tip {
    QString function;
    QString pointer;
    QString object;
    QString dir;
    QString file;
    QString line;
};
class QTreeWidget;
class QTreeWidgetItem;
class XmlStreamReaderPrivate;
class XmlStreamReader : public QObject
{
    Q_OBJECT;
public:
    XmlStreamReader(QTreeWidget *tree);

    bool readFile(const QString &fileName = NULL);

private:
    void readErrorElement();
    void readWhatElement();
    void readXWhatElement();
    void readStackElement(QTreeWidgetItem *parent);
    void readFrameElement(QTreeWidgetItem *parent);
    void skipUnknownElement();

    void saveTip(Tip &tip, const QString &name, const QString &text);
    void setItem(Tip &tip, QTreeWidgetItem *parent);
    QString toolTipContent(Tip &tip);

    QTreeWidget *treeWidget = nullptr;
    QXmlStreamReader reader;
    QString currentIssue;
};

#endif // XMLSTREAMREADER_H
