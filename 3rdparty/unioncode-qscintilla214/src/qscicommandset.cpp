// This module implements the QsciCommandSet class.
//
// Copyright (c) 2023 Riverbank Computing Limited <info@riverbankcomputing.com>
// 
// This file is part of QScintilla.
// 
// This file may be used under the terms of the GNU General Public License
// version 3.0 as published by the Free Software Foundation and appearing in
// the file LICENSE included in the packaging of this file.  Please review the
// following information to ensure the GNU General Public License version 3.0
// requirements will be met: http://www.gnu.org/copyleft/gpl.html.
// 
// If you do not wish to use this file under the terms of the GPL version 3.0
// then you may purchase a commercial license.  For more information contact
// info@riverbankcomputing.com.
// 
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


#include "Qsci/qscicommandset.h"

#include <QSettings>

#include "Qsci/qscicommand.h"
#include "Qsci/qsciscintilla.h"
#include "Qsci/qsciscintillabase.h"


// Starting with QScintilla v2.7 the standard OS/X keyboard shortcuts are used
// where possible.  In order to restore the behaviour of earlier versions then
// #define DONT_USE_OSX_KEYS here or add it to the qmake project (.pro) file.
#if defined(Q_OS_MAC) && !defined(DONT_USE_OSX_KEYS)
#define USING_OSX_KEYS
#else
#undef  USING_OSX_KEYS
#endif


// The ctor.
QsciCommandSet::QsciCommandSet(QsciScintilla *qs) : qsci(qs)
{
    struct sci_cmd {
        QsciCommand::Command cmd;
        int key;
        int altkey;
        const char *desc;
    };

    static struct sci_cmd cmd_table[] = {
        {
            QsciCommand::LineDown,
            Qt::Key_Down,
#if defined(USING_OSX_KEYS)
            Qt::Key_N | Qt::META,
#else
            0,
#endif
            QT_TRANSLATE_NOOP("QsciCommand", "Move down one line")
        },
        {
            QsciCommand::LineUp,
            Qt::Key_Up,
#if defined(USING_OSX_KEYS)
            Qt::Key_P | Qt::META,
#else
            0,
#endif
            QT_TRANSLATE_NOOP("QsciCommand", "Move up one line")
        },
        {
            QsciCommand::Newline,
            Qt::Key_Return,
            Qt::Key_Return | Qt::SHIFT,
            QT_TRANSLATE_NOOP("QsciCommand", "Insert newline")
        },
        {
            QsciCommand::Tab,
            Qt::Key_Tab,
            0,
            QT_TRANSLATE_NOOP("QsciCommand", "Indent one level")
        },
        {
            QsciCommand::Cancel,
            Qt::Key_Escape,
            0,
            QT_TRANSLATE_NOOP("QsciCommand", "Cancel")
        }
    };

    // Clear the default map.
    qsci->SendScintilla(QsciScintillaBase::SCI_CLEARALLCMDKEYS);

    // By default control characters don't do anything (rather than insert the
    // control character into the text).
    for (int k = 'A'; k <= 'Z'; ++k)
        qsci->SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY,
                k + (QsciScintillaBase::SCMOD_CTRL << 16),
                QsciScintillaBase::SCI_NULL);

    for (int i = 0; i < sizeof (cmd_table) / sizeof (cmd_table[0]); ++i)
        cmds.append(
                new QsciCommand(qsci, cmd_table[i].cmd, cmd_table[i].key,
                        cmd_table[i].altkey, cmd_table[i].desc));
}


// The dtor.
QsciCommandSet::~QsciCommandSet()
{
    for (int i = 0; i < cmds.count(); ++i)
        delete cmds.at(i);
}


// Read the command set from settings.
bool QsciCommandSet::readSettings(QSettings &qs, const char *prefix)
{
    bool rc = true;

    for (int i = 0; i < cmds.count(); ++i)
    {
        QsciCommand *cmd = cmds.at(i);

        QString skey = QString("%1/keymap/c%2/").arg(prefix).arg(static_cast<int>(cmd->command()));

        int key;
        bool ok;

        // Read the key.
        ok = qs.contains(skey + "key");
        key = qs.value(skey + "key", 0).toInt();

        if (ok)
            cmd->setKey(key);
        else
            rc = false;

        // Read the alternate key.
        ok = qs.contains(skey + "alt");
        key = qs.value(skey + "alt", 0).toInt();

        if (ok)
            cmd->setAlternateKey(key);
        else
            rc = false;
    }

    return rc;
}


// Write the command set to settings.
bool QsciCommandSet::writeSettings(QSettings &qs, const char *prefix)
{
    bool rc = true;

    for (int i = 0; i < cmds.count(); ++i)
    {
        QsciCommand *cmd = cmds.at(i);

        QString skey = QString("%1/keymap/c%2/").arg(prefix).arg(static_cast<int>(cmd->command()));

        // Write the key.
        qs.setValue(skey + "key", cmd->key());

        // Write the alternate key.
        qs.setValue(skey + "alt", cmd->alternateKey());
    }

    return rc;
}


// Clear the key bindings.
void QsciCommandSet::clearKeys()
{
    for (int i = 0; i < cmds.count(); ++i)
        cmds.at(i)->setKey(0);
}


// Clear the alternate key bindings.
void QsciCommandSet::clearAlternateKeys()
{
    for (int i = 0; i < cmds.count(); ++i)
        cmds.at(i)->setAlternateKey(0);
}


// Find the command bound to a key.
QsciCommand *QsciCommandSet::boundTo(int key) const
{
    for (int i = 0; i < cmds.count(); ++i)
    {
        QsciCommand *cmd = cmds.at(i);

        if (cmd->key() == key || cmd->alternateKey() == key)
            return cmd;
    }

    return 0;
}


// Find a command.
QsciCommand *QsciCommandSet::find(QsciCommand::Command command) const
{
    for (int i = 0; i < cmds.count(); ++i)
    {
        QsciCommand *cmd = cmds.at(i);

        if (cmd->command() == command)
            return cmd;
    }

    // This should never happen.
    return 0;
}
