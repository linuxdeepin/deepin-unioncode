// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTACTION_H // Corrected the header guard name
#define ABSTRACTACTION_H

#include <functional>
#include <QKeySequence>
#include <QAction>

#include <memory> // For using std::unique_ptr

class AbstractAction : public QObject
{
    Q_OBJECT // Add Q_OBJECT macro for signals and slots
public:
    explicit AbstractAction(QAction *qAction, QObject *parent = nullptr);
    virtual ~AbstractAction();

    // Use const references for input parameters
    void setShortCutInfo(const QString &id, const QString &description, 
                         const QKeySequence &defaultShortCut = QKeySequence());

    bool hasShortCut() const;
    QString id() const;
    QString description() const;
    QKeySequence keySequence() const;

    QAction *qAction() const;

private:
    std::unique_ptr<class AbstractActionPrivate> d; // Use std::unique_ptr for automatic memory management
};

#endif // ABSTRACTACTION_H 
