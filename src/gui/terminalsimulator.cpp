#include "terminaledit.h"
#include "terminaldataio.h"

#include "terminalsimulator.h"

TerminalSimulator::TerminalSimulator(QWidget *parent) :
    QDockWidget(parent),
    m_textEdit(new TerminalEdit(this)),
    m_dataIo(new TerminalDataIo(this))
{
    // Set the widget.
    setWindowTitle("Terminal");
    setWidget(m_textEdit);
    m_dataIo->setEditor(m_textEdit);
}

PeanutDataIo *TerminalSimulator::terminalIo() const
{
    return m_dataIo;
}
