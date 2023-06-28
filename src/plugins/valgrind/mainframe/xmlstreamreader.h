// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
