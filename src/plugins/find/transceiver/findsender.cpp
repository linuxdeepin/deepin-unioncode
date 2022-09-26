/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "findsender.h"
#include "framework.h"
#include "common/common.h"

FindSender::FindSender(QObject *parent)
    : QObject(parent)
{

}

FindSender::~FindSender()
{

}

void FindSender::sendSearchCommand(const QString &searchText, int operateType)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_SEARCH);
    event.setProperty(P_SRCTEXT, searchText);
    event.setProperty(P_OPRATETYPE, operateType);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void FindSender::sendReplaceCommand(const QString &searchText, const QString &replaceText, int operateType)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_REPLACE);
    event.setProperty(P_SRCTEXT, searchText);
    event.setProperty(P_DESTTEXT, replaceText);
    event.setProperty(P_OPRATETYPE, operateType);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void FindSender::sendDoubleClickFileInfo(const QString &workspace, const QString &language, const QString &filePath, int line)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_OPENFILE);
    event.setProperty(P_WORKSPACEFOLDER, workspace);
    event.setProperty(P_LANGUAGE, language);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, line);
    dpf::EventCallProxy::instance().pubEvent(event);
}

