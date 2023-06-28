// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROFILESETTINGWIDGET_H
#define PROFILESETTINGWIDGET_H

#include <common/widget/pagewidget.h>
#include <QMap>

static const QString lNameItem{"name"};
static const QString lPathItem{"path"};

using LanguagePaths = QMap<QString, QString>;
class ProfileSettingWidgetPrivate;
class ProfileSettingWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit ProfileSettingWidget(QWidget *parent = nullptr);
    virtual ~ProfileSettingWidget();
    QString translateFilePath();
    QString languageFilePath();
    const LanguagePaths &getLanguagePaths() const;

    void saveConfig() override;
    void readConfig() override;

private:
    void setupUi();
    ProfileSettingWidgetPrivate *const d;

protected:
    void readTranslate();
};

#endif // PROFILESETTINGWIDGET_H
