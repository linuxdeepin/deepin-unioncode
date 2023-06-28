// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xmlstreamreader.h"
#include "valgrind.h"

#include "common/common.h"

#include <QFile>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLabel>

XmlStreamReader::XmlStreamReader(QTreeWidget *tree)
{
    treeWidget = tree;

    connect(treeWidget, &QTreeWidget::itemClicked, [=](QTreeWidgetItem *item, int column) {
        QStringList toolTip = item->toolTip(column).split(":");
        if (toolTip.count() > 1) {
            QString filePath = toolTip.at(0);
            QString line = toolTip.at(1);
            editor.jumpToLine(filePath, line.toInt());
        }
    });
}

bool XmlStreamReader::readFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly))
        return false;

    reader.setDevice(&file);
    reader.readNext();
    while (!reader.atEnd()) {
        QString temp = reader.name().toString();
        if (reader.isStartElement() && reader.name() == "error") {
            readErrorElement();
        } else {
            reader.readNext();
        }
    }

    return true;
}

void XmlStreamReader::readErrorElement()
{
    reader.readNext();
    while (!reader.atEnd()) {
        QString temp = reader.name().toString();
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if ( reader.name() == "what" || reader.name() == "auxwhat") {
                readWhatElement();
            } else if (reader.name() == "xwhat") {
                readXWhatElement();
            } else if (reader.name() == "stack") {
                readStackElement(treeWidget->invisibleRootItem());
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}

void XmlStreamReader::readWhatElement()
{
    currentIssue = reader.readElementText();
    if (reader.isEndElement())
        reader.readNext();
}

void XmlStreamReader::readXWhatElement()
{
    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "text") {
                currentIssue = reader.readElementText();
                if (reader.isEndElement())
                    reader.readNext();
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}

void XmlStreamReader::readStackElement(QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, currentIssue);

    reader.readNext();
    while (!reader.atEnd()) {
        QString temp = reader.name().toString();
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "frame") {
                readFrameElement(item);
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}

void XmlStreamReader::readFrameElement(QTreeWidgetItem *parent)
{
    Tip tip;

    reader.readNext();
    while (!reader.atEnd()) {
        QString temp = reader.name().toString();
        if (reader.isEndElement()) {
            if (reader.name() == "frame") {
                reader.readNext();
                break;
            }
            reader.readNext();
        }

        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            QString text = reader.readElementText();
            saveTip(tip, name, text);
        } else {
            reader.readNext();
        }
    }

    setItem(tip, parent);
}

void XmlStreamReader::skipUnknownElement()
{
    reader.readNext();
    while (!reader.atEnd()) {
        QString temp = reader.name().toString();
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            skipUnknownElement();
        } else {
            reader.readNext();
        }
    }
}

void XmlStreamReader::saveTip(Tip &tip, const QString &name, const QString &text)
{
    if (name == "ip") {
        tip.pointer = text;
    } else if (name == "obj") {
        tip.object = text;
    } else if (name == "fn") {
        tip.function = text;
    } else if (name == "dir") {
        tip.dir = text;
    } else if (name == "file") {
        tip.file = text;
    } else if (name == "line") {
        tip.line = text;
    }

    return ;
}

void XmlStreamReader::setItem(Tip &tip, QTreeWidgetItem *parent)
{
    if (!tip.function.isEmpty() && !tip.line.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        QString filePath = tip.dir + "/" + tip.file;
        item->setText(0, tip.function);
        item->setToolTip(0, toolTipContent(tip));
        item->setForeground(1, QColor("green"));
        item->setText(1, tip.function + ":" + tip.line + ":0");
        item->setToolTip(1, filePath + ":" + tip.line + ":0");
    } else if (!tip.function.isEmpty() && !tip.object.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, tip.function + " in " + tip.object);
        item->setToolTip(0, toolTipContent(tip));
    }
}

QString XmlStreamReader::toolTipContent(Tip &tip)
{
    QString toolTip = tr("Function:\n\t") + tip.function + "\n" + tr("Instruction Pointer:\n\t") +
            tip.pointer + "\n" + tr("Object:\n\t") + tip.object;
    if (!tip.dir.isEmpty()) {
        toolTip += "\n" + tr("Location:\n\t") + tip.dir + tip.file + ":" + tip.line;
    }

    return toolTip;
}

