// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OUTPUTDOCUMENTFIND_H
#define OUTPUTDOCUMENTFIND_H

#include "abstractdocumentfind.h"

class OutputPane;
class OutputDocumentFindPrivate;
class OutputDocumentFind : public AbstractDocumentFind
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
