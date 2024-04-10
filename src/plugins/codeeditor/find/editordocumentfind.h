// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORDOCUMENTFIND_H
#define EDITORDOCUMENTFIND_H

#include "base/abstractdocumentfind.h"

class EditorDocumentFindPrivate;
class EditorDocumentFind : public AbstractDocumentFind
{
    Q_OBJECT
public:
    explicit EditorDocumentFind(QObject *parent = nullptr);

    virtual QString findString() const override;
    virtual void findNext(const QString &txt) override;
    virtual void findPrevious(const QString &txt) override;
    virtual void replace(const QString &before, const QString &after) override;
    virtual void replaceFind(const QString &before, const QString &after) override;
    virtual void replaceAll(const QString &before, const QString &after) override;
    virtual void findStringChanged() override;

private:
    EditorDocumentFindPrivate *const d;
};

#endif // EDITORDOCUMENTFIND_H
