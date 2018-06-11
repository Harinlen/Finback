#include "terminaledit.h"

#include "terminaldataio.h"

#include <QDebug>

TerminalDataIo::TerminalDataIo(QObject *parent) : PeanutDataIo(parent),
    m_editor(nullptr),
    m_clearTerminal(true)
{
}

QChar TerminalDataIo::read()
{
    if(m_readCache.isEmpty())
    {
        return QChar();
    }
    QChar data = m_readCache.at(0).toLatin1();
    m_readCache=m_readCache.mid(1);
    return data;
}

bool TerminalDataIo::isCacheEmpty()
{
    return m_readCache.isEmpty();
}

void TerminalDataIo::setEditor(TerminalEdit *editor)
{
    if(m_editor)
    {
        disconnect(m_editor, 0, 0, 0);
    }
    m_editor = editor;
    if(m_editor)
    {
        connect(m_editor, &TerminalEdit::dataIn, [=](char data)
        {
            // Append the data cache.
            m_readCache.append(data);
        });
    }
}

void TerminalDataIo::resetCache()
{
    m_readCache = QString();
    // Check editor clear option.
    if(m_clearTerminal && m_editor)
    {
        ;
    }
}

void TerminalDataIo::write(QChar data)
{
    if(m_editor)
    {
        m_editor->moveCursor(QTextCursor::End);
        m_editor->insertPlainText(data);
        m_editor->moveCursor(QTextCursor::End);
    }
}
