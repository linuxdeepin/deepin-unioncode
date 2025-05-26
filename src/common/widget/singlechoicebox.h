// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLECHOICEBOX_H
#define SINGLECHOICEBOX_H

#include <DWidget>

#include <QGroupBox>
#include <QIcon>

class SingleChoiceBoxPrivate;
class SingleChoiceBox : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
    SingleChoiceBoxPrivate *const d;
public:
    struct Info {
        QString text;
        QString tooltip;
        QIcon icon;
        inline bool operator == (const Info& info) const {
            return text == info.text
                    && tooltip == info.tooltip;
        }

        inline Info& operator=(const Info& info) {
            text = info.text;
            tooltip = info.tooltip;
            icon = info.icon;
            return *this;
        }
    };

    explicit SingleChoiceBox(QWidget *parent = nullptr);
    virtual ~SingleChoiceBox();
    void setInfos(QSet<Info> infos);
    void setChoiceTitle(const QString &title);

signals:
    void selected(const Info &info);

private slots:
    void toggled(bool checked);
};

uint qHash(const SingleChoiceBox::Info &info, uint seed = 0);

#endif // SINGLECHOICEBOX_H
