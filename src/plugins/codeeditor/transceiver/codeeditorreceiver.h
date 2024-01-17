// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void openFileRequested(const QString &fileName);

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
    void toCleanAllAnnotation(const QString &title);
    void toSwitchContext(const QString &name);
    void toSwitchWorkspace(const QString &name);
    void toSetModifiedAutoReload(const QString filePath, bool flag);
    void toAddDebugPoint(const QString &filePath, int line);
    void toRemoveDebugPoint(const QString &filePath, int line);
};

#endif // CODEEDITORRECEIVER_H
