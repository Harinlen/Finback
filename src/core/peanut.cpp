#include <QApplication>

#include "peanutcpu.h"
#include "peanutmemory.h"

#include "peanut.h"

#include <QDebug>

Peanut::Peanut(QObject *parent) : QObject(parent),
    m_cpu(new PeanutCpu(this)),
    m_memory(new PeanutMemory(this)),
    m_stop(false)
{
    // Set the memory to cpu.
    m_cpu->setMemory(m_memory);

    connect(this, &Peanut::requireNext, this, &Peanut::execute,
            Qt::QueuedConnection);
}

void Peanut::execute()
{
    // Check stop.
    if(m_stop)
    {
        return;
    }
    //Execute one step.
    step();
    // Process event.
    QApplication::processEvents();
    // Emit the next executing.
    // Check stop.
    if(!m_cpu->isHalt() && !m_stop)
    {
        emit requireNext();
    }
}

void Peanut::step()
{
    //Check stop.
    if(m_stop)
    {
        return;
    }
    // Check the halt state.
    if(m_cpu->isHalt())
    {
        emit halt();
        return;
    }
    // Execute next instruction.
    m_cpu->executeNext();
}

PeanutMemory *Peanut::memory() const
{
    return m_memory;
}

void Peanut::setDataIo(PeanutDataIo *dataIo)
{
    m_memory->setDataIo(dataIo);
}

void Peanut::reset()
{
    m_stop = false;
    m_cpu->reset();
    m_memory->reset();
}
