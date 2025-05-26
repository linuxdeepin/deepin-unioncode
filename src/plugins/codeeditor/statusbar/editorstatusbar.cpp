// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorstatusbar.h"
#include "encodecombobox.h"

#include <DFrame>

#include <QLabel>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class EditorStatusBarPrivate : public QObject
{
public:
    explicit EditorStatusBarPrivate(EditorStatusBar *qq);

    void initUI();
    void initConnection();

public:
    EditorStatusBar *q;

    QLabel *cursorlabel { nullptr };
    QLabel *modeLabel { nullptr };
    EncodeComboBox *encodedCB { nullptr };
};

EditorStatusBarPrivate::EditorStatusBarPrivate(EditorStatusBar *qq)
    : q(qq)
{
}

void EditorStatusBarPrivate::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    auto addVLine = [this, layout] {
        DVerticalLine *vLine = new DVerticalLine(q);
        vLine->setFixedHeight(14);
        layout->addWidget(vLine);
    };

    cursorlabel = new QLabel(q);
    modeLabel = new QLabel(q);
    encodedCB = new EncodeComboBox(q);

    layout->addWidget(cursorlabel);
    addVLine();
    layout->addWidget(modeLabel);
    addVLine();
    layout->addWidget(encodedCB);
}

void EditorStatusBarPrivate::initConnection()
{
    connect(encodedCB, &EncodeComboBox::encodeChanged, q, &EditorStatusBar::encodeChanged);
}

EditorStatusBar::EditorStatusBar(QWidget *parent)
    : QWidget(parent),
      d(new EditorStatusBarPrivate(this))
{
    d->initUI();
    d->initConnection();
}

EditorStatusBar::~EditorStatusBar()
{
    delete d;
}

void EditorStatusBar::updateCursor(int line, int column)
{
    QString format(tr("Line %1 Column %2"));
    d->cursorlabel->setText(format.arg(line + 1).arg(column + 1));
}

void EditorStatusBar::updateCursorMode(CursorMode mode)
{
    QString text;
    switch (mode) {
    case Insert:
        text = tr("INSERT");
        break;
    case Overwrite:
        text = tr("OVERWRITE");
        break;
    case Readonly:
        text = tr("R/O");
        break;
    }
    d->modeLabel->setText(text);
}

void EditorStatusBar::updateEncodedMode(const QString &mode)
{
    d->encodedCB->setEncodeName(mode);
}
