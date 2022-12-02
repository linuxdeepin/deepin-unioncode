/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
