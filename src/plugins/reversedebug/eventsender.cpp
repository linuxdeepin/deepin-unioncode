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
#include "eventsender.h"
#include "framework.h"
#include "common/common.h"

void EventSender::jumpTo(const QString &filePath, int lineNum)
{
    if (filePath.isEmpty() || lineNum < 0)
        return;

    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_JUMP_TO_LINE);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, lineNum);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void EventSender::setAnnotation(const QString &filePath, int lineNum, const QString &text)
{
    if (filePath.isEmpty() || lineNum < 0 || text.isEmpty())
        return;

    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_SET_ANNOTATION);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, lineNum);
    event.setProperty(P_TEXT, text);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void EventSender::cleanAnnotation(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_CLEAN_ANNOTATION);
    event.setProperty(P_FILEPATH, filePath);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void EventSender::setLineBackground(const QString &filePath, QColor &color, int lineNum)
{
    if (filePath.isEmpty() || lineNum < 0)
        return;

    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_SET_LINE_BACKGROUND);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, lineNum);
    event.setProperty(P_COLOR, color);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void EventSender::cleanLineBackground(const QString &filePath, int lineNum)
{
    if (filePath.isEmpty() || lineNum < 0)
        return;

    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_DEL_LINE_BACKGROUND);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, lineNum);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void EventSender::cleanLineBackground(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_CLEAN_LINE_BACKGROUND);
    event.setProperty(P_FILEPATH, filePath);
    dpf::EventCallProxy::instance().pubEvent(event);
}

EventSender::EventSender(QObject *parent) : QObject(parent)
{

}
