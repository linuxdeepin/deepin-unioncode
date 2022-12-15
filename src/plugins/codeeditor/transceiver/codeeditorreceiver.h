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
#ifndef CODEEDITORRECEIVER_H
#define CODEEDITORRECEIVER_H

#include "common/common.h"
#include "framework.h"

class CodeEditorReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<CodeEditorReceiver>
{
    friend class dpf::AutoEventHandlerRegister<CodeEditorReceiver>;
public:
    explicit CodeEditorReceiver(QObject * parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event) override;
};

class EditorCallProxy : public QObject
{
    Q_OBJECT
    EditorCallProxy(const EditorCallProxy&) = delete;
    EditorCallProxy();

public:
    static EditorCallProxy* instance();

signals:
    void toOpenFile(const QString &filePath);
    void toRunClean();
    void toDebugPointClean();
    void toSearchText(const QString &srcText, int operateType);
    void toReplaceText(const QString &srcText, const QString &destText, int operateType);
    void toOpenFileWithKey(const newlsp::ProjectKey &key, const QString &filePath);
    void toRunFileLineWithKey(const newlsp::ProjectKey &key, const QString &filePath, int line);
    void toJumpFileLineWithKey(const newlsp::ProjectKey &key, const QString &filePath, int line);
    void toSetLineBackground(const QString &filePath, int line, const QColor &color);
    void toDelLineBackground(const QString &filePath, int line);
    void toCleanLineBackground(const QString &filePath);
    void toSetAnnotation(const QString &filePath, int line, const QString &title, const AnnotationInfo &info);
    void toCleanAnnotation(const QString &filePath, const QString &title);
    void toSetAnalysedData(const AnalysedData &data);
    void toSwitchContext(const QString &name);
    void toSwitchWorkspace(const QString &name);
    void toSetModifiedAutoReload(const QString filePath, bool flag);
};

#endif // CODEEDITORRECEIVER_H
