#include <QEvent>
#include <QKeyEvent>

#include "terminaledit.h"

TerminalEdit::TerminalEdit(QWidget *parent) : QPlainTextEdit(parent)
{
}

bool TerminalEdit::event(QEvent *e)
{
    if(e->type() == QEvent::KeyPress)
    {
        QString keyText = static_cast<QKeyEvent *>(e)->text();
        // Emit a new type signal.
        if(!keyText.isEmpty())
        {
            emit dataIn(keyText.at(0).toLatin1());
        }
        e->accept();
        return true;
    }
    return QPlainTextEdit::event(e);
}
