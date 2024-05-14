// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSCONFIGVIEW_H
#define BINARYTOOLSCONFIGVIEW_H

#include <DWidget>

class BinaryToolsConfigViewPrivate;
class BinaryToolsConfigView : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit BinaryToolsConfigView(QWidget *parent = nullptr);
    ~BinaryToolsConfigView();

    bool saveConfig();

protected:
    // bool eventFilter(QObject *obj, QEvent *e) override;

private:
    BinaryToolsConfigViewPrivate *const d = nullptr;
    friend class BinaryToolsConfigViewPrivate;
};

#endif   // BINARYTOOLSCONFIGVIEW_H
