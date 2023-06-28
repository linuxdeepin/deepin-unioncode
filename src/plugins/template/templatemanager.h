// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

#include <QObject>

class TemplateManagerPrivate;
class TemplateManager : public QObject
{
    Q_OBJECT
public:
    static TemplateManager *instance();
    void initialize();

signals:

public slots:
    void newWizard();

private:
    explicit TemplateManager(QObject *parent = nullptr);
    ~TemplateManager();

    void addMenu();

    TemplateManagerPrivate *const d;
};

#endif // TEMPLATEMANAGER_H

