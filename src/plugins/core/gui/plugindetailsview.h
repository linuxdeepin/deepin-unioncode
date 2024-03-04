// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PLUGINDETAILSVIEW_H
#define PLUGINDETAILSVIEW_H

#include "framework/lifecycle/pluginmetaobject.h"
#include <dtkwidget_global.h>

#include <DWidget>
#include <DListWidget>

QT_BEGIN_NAMESPACE
class QGridLayout;
class QVBoxLayout;
class QSpacerItem;
class QWebEngineView;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DTextEdit;
class DLabel;
DWIDGET_END_NAMESPACE
class DetailsView : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    struct MetaInfo
    {
        QString name;
        QString version;
        QString compatibleVersion;
        QString vendor;
        QString copyright;
        QString category;
        QString url;
        QString license;
        QString description;
        QString dependency;

        QString toHtml()
        {
            QString html = "<html>"
                              "<body>"
                              "<table>"
                                  "<tr><td><strong>" + tr("Name") + "：</strong></td><td>" + name + "</td></tr>"
                                  "<tr><td><strong>" + tr("Version") + "：</strong></td><td>" + version + "</td></tr>"
                                  "<tr><td><strong>" + tr("Compatible Version") + "：</strong></td><td>" + compatibleVersion + "</td></tr>"
                                  "<tr><td><strong>" + tr("Vendor") + "：</strong></td><td>" + vendor + "</td></tr>"
//                                  "<tr><td><strong>" + tr("Copyright") + "：</strong></td><td>" + copyright + "</td></tr>"
                                  "<tr><td><strong>" + tr("Category") + "：</strong></td><td>" + category + "</td></tr>"
//                                  "<tr><td><strong>" + tr("URL") + "：</strong></td><td>" + url + "</td></tr>"
                                  "<tr><td><strong>" + tr("License") + "：</strong></td><td>" + license + "</td></tr>"
                                  "<tr><td><strong>" + tr("Description") + "：</strong></td><td>" + description + "</td></tr>"
                                  "<tr><td><strong>" + tr("Dependency") + "：</strong></td><td>" + dependency + "</td></tr>"
                              "</table><br>"
                              "</body>"
                              "</html>";
            return html;
        }
    };

    explicit DetailsView(QWidget *parent = nullptr);
    ~DetailsView();

    void update(const dpf::PluginMetaObjectPointer &metaInfo);

private slots:
    void changeLoadBtnState();
    void showCfgWidget();

private:
    void setupUi();
    void updateLoadBtnDisplay(bool isEnabled);

    DTK_WIDGET_NAMESPACE::DLabel *metaInfoLabel {nullptr};
    DTK_WIDGET_NAMESPACE::DPushButton *loadBtn {nullptr};
    dpf::PluginMetaObjectPointer pluginMetaInfo;
    QWebEngineView *webView {nullptr};
};

#endif // PLUGINDETAILSVIEW_H
