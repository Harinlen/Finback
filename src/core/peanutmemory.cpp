#include "peanutdisplay.h"
#include "peanutdataio.h"

#include "peanutmemory.h"

#include <QDebug>

PeanutMemory::PeanutMemory(QObject *parent) : QObject(parent),
    m_display(nullptr),
    m_dataIo(nullptr)
{
}

bool PeanutMemory::isInputEmpty()
{
    if(m_dataIo)
    {
        return m_dataIo->isCacheEmpty();
    }
    // When no data io, always true.
    return true;
}

bool PeanutMemory::read(PEANUT_ADDRESS_TYPE address, PEANUT_WORD_TYPE *target)
{
    // Read the memory data.
    if(address==0xFFF0)
    {
        // Input the data.
        if(m_dataIo)
        {
            // Read the data from the terminal.
            (*target) = m_dataIo->read().toLatin1();
        }
        else
        {
            (*target) = 0x00000000;
        }
    }
    else if(address==0xFFF1)
    {
        // Status register.
        if(m_dataIo)
        {
            (*target) = (m_dataIo->isCacheEmpty()) ? 0x00000000 : 0x80000000;
        }
        else
        {
            (*target) = 0x40000000;
        }
    }
    else
    {
        // Normal address.
        (*target) = m_memoryBank.value(address, PEANUT_WORD_NULL);
    }
    return true;
}

bool PeanutMemory::write(PEANUT_ADDRESS_TYPE address, PEANUT_WORD_TYPE value)
{
    // Check special address.
    if(address==0xFFF0)
    {
        // Print out the data.
        if(m_dataIo)
        {
            qDebug()<<(char)value;
            m_dataIo->write((char)value);
        }
    }
    else if(address==0xFFF1)
    {
        // Read only memory.
        return false;
    }
    else if(address>=0x7C40 && address<=0x7FFF)
    {
        // Display hardware.
        // Set the value to the memory.
        m_memoryBank.insert(address, value);
        // Also tell the display to update specific area.
        if(m_display)
        {
            m_display->updatePixels(address - 0x7C40, value);
        }
    }
    else
    {
        // General memory write.
        // Set the value to the memory.
        m_memoryBank.insert(address, value);
    }
    return true;
}

void PeanutMemory::reset()
{
    // Loop and reset all the data
    m_memoryBank = QHash<PEANUT_ADDRESS_TYPE, PEANUT_WORD_TYPE>();
    // Check data io.
    if(m_dataIo)
    {
        m_dataIo->resetCache();
    }
}

void PeanutMemory::setDisplay(PeanutDisplay *display)
{
    m_display = display;
}

void PeanutMemory::setDataIo(PeanutDataIo *dataIo)
{
    m_dataIo = dataIo;
}
