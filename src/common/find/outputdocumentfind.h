// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OUTPUTDOCUMENTFIND_H
#define OUTPUTDOCUMENTFIND_H

#include "common/common_global.h"
#include "abstractdocumentfind.h"

class OutputPane;
class OutputDocumentFindPrivate;
class COMMON_EXPORT OutputDocumentFind : public AbstractDocumentFind
{
    Q_OBJECT
public:
    explicit OutputDocumentFind(OutputPane *parent);
    ~OutputDocumentFind();

    virtual QString findString() const override;
    virtual void findNext(const QString &txt) override;
    virtual void findPrevious(const QString &txt) override;
    virtual bool supportsReplace() const override;

private:
    OutputDocumentFindPrivate *const d;
};

#endif   // OUTPUTDOCUMENTFIND_H
