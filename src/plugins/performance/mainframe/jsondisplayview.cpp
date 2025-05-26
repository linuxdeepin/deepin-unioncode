// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsondisplaymodel.h"
#include "jsondisplayview.h"
#include "jsondispalysfmodel.h"

#include <QSortFilterProxyModel>
class JsonDisplayViewPrivate
{
    friend class JsonDisplayView;
    JsonDisplayModel *model{nullptr};
    JsonDispalySFModel *sfModel{nullptr};
};

JsonDisplayView::JsonDisplayView(QWidget *parent)
    : QTableView (parent)
    , d (new JsonDisplayViewPrivate)
{
    d->model = new JsonDisplayModel(this);
    d->sfModel = new JsonDispalySFModel(this);
    d->sfModel->setSourceModel(d->model);
    setSelectionMode(SelectionMode::SingleSelection);
    setSelectionBehavior(SelectionBehavior::SelectRows);
    setSortingEnabled(true);
    setModel(d->sfModel);
}

JsonDisplayView::~JsonDisplayView()
{
    if (d)
        delete d;
}

void JsonDisplayView::parseJson(const Json::Value &jsonObj)
{
    setUpdatesEnabled(false);
    d->model->parseJson(jsonObj);
    setUpdatesEnabled(true);
}
